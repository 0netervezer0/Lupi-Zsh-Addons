// Lupi Zsh Addons V0.4.2
// * Main File * //

#include "functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

#define FLAG_ERR   RED     "[X]" RESET
#define FLAG_WARN  YELLOW  "[!]" RESET
#define FLAG_INFO  BLUE    "[?]" RESET
#define FLAG_OK    GREEN    "[✓]" RESET


// main function
int main( int argc, char* argv[] ) {
    if ( argc < 2 ) {
        fprintf( stderr, "%s Lupi Zsh Addons v0.4.2\nUse:\n"
            "  Default commands:\n"
            "   help - the command list\n"
            "   cache - the terminal cache size and clean it\n"
            "   new - start new terminal session\n"
            "   newc - start new terminal session in current directory\n"
            "   space - show disk usage of home directory\n"
            "\n"
            "  Environment commands:\n"
            "   hist - the command history of your terminal\n"
            "   Usage: rc [option] - show or edit .zshrc\n"
            "     Options:\n"
            "      view - show .zshrc contents\n"
            "      edit - edit .zshrc using nano\n"
            "   Usage: script [option] [script name]\n"
            "     Options:\n"
            "      edit [script name] - edit your script using nano\n"
            "      delete [script name] - remove your script\n"
            "      list - return a list of your scripts\n"
            "     Usage: script create [script name] [.sh file path]\n"
            "      - сreates a command that executes your script\n"
            "        (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n", FLAG_WARN );
        return 1;
    }

    // commands
    const char* cmd = argv[1];

    if ( strcmp( cmd, "space" ) == 0 ) {
        show_disk_space();

    } else if ( strcmp( cmd, "script" ) == 0 ) {
        if ( argc >= 3 ) {
            if ( strcmp( argv[2], "edit" ) == 0 ) {
                if ( argc == 4 ) {
                    script_edit( argv[3] );
                } else {
                    fprintf( stderr, "%s Usage: script [option] [script name]\n"
                    "     Options:\n"
                    "      edit - edit your script using nano\n"
                    "      delete - remove your script\n", FLAG_WARN );
                }
            } else if ( strcmp( argv[2], "delete" ) == 0 ) {
                if ( argc == 4 ) {
                    script_delete( argv[3] );
                } else {
                    fprintf( stderr, "%s Usage: script [option] [script name]\n"
                    "     Options:\n"
                    "      edit - edit your script using nano\n"
                    "      delete - remove your script\n", FLAG_WARN );
                }
            } else if ( strcmp( argv[2], "create" ) == 0 ) {
                if ( argc == 5 ) {
                    script_create( argv[3], argv[4] );
                } else if ( argc == 4 ) {
                    script_create_empty( argv[3] );
                } else {
                    fprintf( stderr, "%s Usage: script create [script name] [.sh file path]\n"
                    "     - сreates a command that executes your script\n"
                    "       (https://github.com/0netervezer0/Lupi-Zsh-Addons/README.md for more)\n",
                    FLAG_WARN );
                }
            } else if ( strcmp( argv[2], "list" ) == 0 ) {
                script_list();
            } else {
                fprintf( stderr, "%s Usage: script create [script name] [.sh file path]\n"
                "     - сreates a command that executes your script\n"
                "       (https://github.com/0netervezer0/Lupi-Zsh-Addons/README.md for more)\n",
                FLAG_WARN );
            }
        } else { fprintf( stderr, "%s Not enough arguments! Use 'help'\n", FLAG_ERR ); }

    } else if ( strcmp( cmd, "rc" ) == 0 ) {
        if ( argc == 3 ) {
            if ( strcmp( argv[2], "view" ) == 0 ) {
                view_zshrc();
            } else if ( strcmp( argv[2], "edit" ) == 0 ) {
                edit_zshrc();
            } else {
                fprintf( stderr, "%s Usage: lupi rc [option] \n"
                "  Options:\n"
                "   view - show .zshrc contents\n"
                "   edit - edit .zshrc using nano\n", FLAG_WARN );
            }
        } else {
            fprintf( stderr, "%s Usage: lupi rc [option] \n"
                "  Options:\n"
                "   view - show .zshrc contents\n"
                "   edit - edit .zshrc using nano\n", FLAG_WARN );
        }

    } else if ( strcmp( cmd, "new" ) == 0 ) {
        start_terminal_session();

    } else if ( strcmp( cmd, "newc" ) == 0 ) {
        start_terminal_session_current_directory();

    } else if ( strcmp( cmd, "hist" ) == 0 ) {
        const char* home_dir = getenv( "HOME" );
        if ( home_dir == NULL ) {
            fprintf( stderr, "%s Can't find HOME user path\n", FLAG_ERR );
            return 1;
        }

        char filepath[1024];
        snprintf( filepath, sizeof( filepath ), "%s/.zsh_history", home_dir );

        FILE* file = fopen( filepath, "r" );
        if ( file == NULL ) {
            fprintf( stderr, "%s Can't find .zsh_history file", FLAG_ERR );
            return 1;
        }
        fclose( file );

        printf( "Your command history: \n" );
        print_file_contents( filepath );

    } else if ( strcmp( cmd, "cache" ) == 0 ) {
        const char* homeDir = getenv( "HOME" );
        if ( homeDir == NULL ) {
            fprintf( stderr, "%s Can't find HOME user path\n", FLAG_ERR );
            return 1;
        }

        char sessionsDir[1024];
        snprintf( sessionsDir, sizeof( sessionsDir ), "%s/.zsh_sessions", homeDir );

        struct stat statbuf;
        if ( stat( sessionsDir, &statbuf ) == -1 || !S_ISDIR( statbuf.st_mode )) {
            fprintf( stderr, "%s Can't find .zsh_sessions folder\n", FLAG_ERR );
            return 1;
        }

        long dirSize = calculate_directory_size( sessionsDir );
        if ( dirSize == -1 ) {
            fprintf( stderr, "%s Can't calculate size of .zsh_sessions\n", FLAG_ERR );
            return 1;
        }

        char zshHistoryPath[1024];
        snprintf( zshHistoryPath, sizeof( zshHistoryPath ), "%s/.zsh_history", homeDir );

        long zshHistorySize = 0;
        if ( stat( zshHistoryPath, &statbuf ) != -1 ) {
            zshHistorySize = statbuf.st_size;
        }

        long totalCacheSize = dirSize + zshHistorySize;
        printf( "%s Cache size: %ld bytes\n", FLAG_WARN, totalCacheSize );

        printf( "%s Clean cache? (y/n): ", FLAG_INFO, RESET );
        char response;
        scanf( " %c", &response );

        if ( response == 'y' || response == 'Y' ) {
            clear_directory( sessionsDir );
            clear_zsh_history( homeDir );
            printf( "%s Cache cleaned\n", FLAG_OK, RESET );
        } else {
            printf( "Clean cancelled\n" );
        }

    } else if ( strcmp( cmd, "help" ) == 0 ) {
        fprintf( stderr, "Use:\n"
            "  Default commands:\n"
            "   help - the command list\n"
            "   cache - the terminal cache size and clean it\n"
            "   new - start new terminal session\n"
            "   newc - start new terminal session in current directory\n"
            "   space - show disk usage of home directory\n"
            "\n"
            "  Environment commands:\n"
            "   hist - the command history of your terminal\n"
            "   Usage: rc [option] - show or edit .zshrc\n"
            "     Options:\n"
            "      view - show .zshrc contents\n"
            "      edit - edit .zshrc using nano\n"
            "   Usage: script [option] [script name]\n"
            "     Options:\n"
            "      edit [script name] - edit your script using nano\n"
            "      delete [script name] - remove your script\n"
            "      list - return a list of your scripts\n"
            "     Usage: script create [script name] [.sh file path]\n"
            "      - сreates a command that executes your script\n"
            "        (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n" );

    } else {
        // Try to execute user script from ~/my scripts
        const char* homeDir = getenv( "HOME" );
        if ( homeDir ) {
            char scriptPath[1024];
            snprintf( scriptPath, sizeof( scriptPath ), "%s/my scripts/%s.sh", homeDir, cmd );

            if ( access( scriptPath, X_OK ) == 0 ) {
                char fullCmd[2048] = { 0 };
                strcat( fullCmd, "\"" );
                strcat( fullCmd, scriptPath );
                strcat( fullCmd, "\"" );
                // Append additional arguments if any
                for ( int i = 2; i < argc; ++i ) {
                    strcat( fullCmd, " \"" );
                    strcat( fullCmd, argv[i] );
                    strcat( fullCmd, "\"" );
                }
                system( fullCmd );
                return 0;
            }
        }

        fprintf( stderr, "%s Unknown argument '%s'. Use 'help' to see the command list\n", FLAG_ERR, cmd );
        return 1;
    }

    return 0;
}
