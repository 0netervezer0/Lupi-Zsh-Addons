// Lupi Zsh Addons V0.4.2

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

#define FLAG_ERR   RED "[X]" RESET
#define FLAG_WARN  YELLOW "[!]" RESET
#define FLAG_INFO  BLUE "[?]" RESET
#define FLAG_OK    GREEN "[✓]" RESET


// Get OS type (macOS/Linux)
const char* get_os_type() {
    #ifdef __APPLE__
        return "macOS";
    #else
        return "Linux";
    #endif
}

// Print calendar of month
void print_calendar( int monthOverride ) {
    time_t t = time( NULL );
    struct tm *currentTime = localtime( &t );

    int day = currentTime -> tm_mday;
    int month = ( monthOverride == 0 ) ? currentTime->tm_mon + 1 : monthOverride;
    int year = currentTime -> tm_year + 1900;

    struct tm firstDay = *currentTime;
    firstDay.tm_mday = 1;
    firstDay.tm_mon = month - 1;
    mktime( &firstDay );

    int startDay = firstDay.tm_wday;

    int daysInMonth;
    if ( month == 2 ) {
        if (( year % 400 == 0) || ( year % 100 != 0 && year % 4 == 0 )) {
            daysInMonth = 29;
        } else {
            daysInMonth = 28;
        }
    } else if ( month == 4 || month == 6 || month == 9 || month == 11 ) {
        daysInMonth = 30;
    } else {
        daysInMonth = 31;
    }

    const char *MONTH_NAMES[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    printf( "\n    %s %d\n", MONTH_NAMES[month - 1], year );
    printf(  "Su Mo Tu We Th Fr Sa\n" );

    for ( int i = 0; i < startDay; i++ ) {
        printf( "   " );
    }

    for ( int d = 1; d <= daysInMonth; d++ ) {
        if ( d == day && monthOverride == 0 ) {
            printf( "\033[1;31m%2d\033[0m ", d );
        } else {
            printf( "%2d ", d );
        }

        if (( startDay + d ) % 7 == 0 || d == daysInMonth ) {
            printf( "\n" );
        }
    }
    printf( "\n" );
}

// Open new terminal session
void restart_terminal_new_session() {
    const char* osType = get_os_type();

    if ( strcmp( osType, "macOS" ) == 0) {
        system( "open -a Terminal ~" );
    } else {
        system( "gnome-terminal -- bash -c 'exec bash'" );
    }

    printf( "New session started\n" );
}

// Open new terminal session in current directory
void restart_terminal_current_directory() {
    const char* osType = get_os_type();
    char cwd[1024];

    if ( getcwd( cwd, sizeof( cwd )) == NULL ) {
        fprintf( stderr, "%s Can't get current directory", FLAG_ERR );
        return;
    }

    if ( strcmp( osType, "macOS" ) == 0 ) {
        char command[2048];
        snprintf( command, sizeof( command ), "open -a Terminal \"%s\"", cwd );
        system( command );
    } else {
        char command[2048];
        snprintf( command, sizeof( command ), "gnome-terminal -- bash -c 'cd %s; exec bash'", cwd );
        system( command );
    }

    printf( "New session started in current directory\n" );
}

// Calculate directory size
long calculate_directory_size( const char* path ) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    long totalSize = 0;

    if (( dir = opendir( path )) == NULL ) {
        fprintf( stderr, "%s Can't find path", FLAG_ERR );
        return -1;
    }


    while (( entry = readdir( dir )) != NULL ) {
        if ( strcmp( entry -> d_name, "." ) == 0 || strcmp( entry -> d_name, ".." ) == 0 ) {
            continue;
        }

        char fullPath[1024];
        snprintf( fullPath, sizeof( fullPath ), "%s/%s", path, entry -> d_name );

        if ( stat( fullPath, &statbuf ) == -1 ) {
            fprintf( stderr, "%s Can't get file information", FLAG_ERR );
            continue;
        }

        if ( S_ISDIR( statbuf.st_mode )) {
            long dir_size = calculate_directory_size( fullPath );
            if ( dir_size == -1 ) {
                closedir( dir );
                return -1;
            }
            totalSize += dir_size;
        } else {
            totalSize += statbuf.st_size;
        }
    }

    closedir( dir );
    return totalSize;
}

// Delete files in directory
void clear_directory( const char* path ) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;

    if (( dir = opendir( path )) == NULL ) {
        fprintf( stderr, "%s Can't open folder", FLAG_ERR );
        return;
    }

    while (( entry = readdir(dir)) != NULL ) {
        if ( strcmp( entry -> d_name, "." ) == 0 || strcmp( entry -> d_name, ".." ) == 0 ) {
            continue;
        }

        char fullPath[1024];
        snprintf( fullPath, sizeof( fullPath ), "%s/%s", path, entry -> d_name );

        if ( stat( fullPath, &statbuf ) == -1 ) {
            fprintf( stderr, "%s Can't get file include", FLAG_ERR );
            continue;
        }

        if ( S_ISDIR( statbuf.st_mode )) {
            clear_directory( fullPath );
            rmdir( fullPath ); 
        } else {
            if ( unlink( fullPath) == -1 ) {
                fprintf( stderr, "%s Can't delete files", FLAG_ERR );
            }
        }
    }
}

// Clear .zsh_history file
void clear_zsh_history( const char* homeDir ) {
    char path[1024];
    snprintf( path, sizeof( path ), "%s/.zsh_history", homeDir );

    FILE* file = fopen( path, "w" );
    if ( file != NULL ) {
        fclose( file );
    } else {
        fprintf( stderr, "%s Can't clear .zsh_history", FLAG_ERR );
    }
}

// Print content in file 
void print_file_contents( const char* filepath ) {
    FILE* file = fopen( filepath, "r" );
    if ( file == NULL ) {
        fprintf( stderr, "%s Can't open file", FLAG_ERR );
        return;
    }

    char buffer[1024];
    while ( fgets( buffer, sizeof( buffer ), file ) != NULL ) {
        printf( "%s", buffer );
    }

    fclose( file );
}

// Disk space (df -h ~)
void show_disk_space() {
    system( "df -h ~" );
}

// Edit the info in .zshrc file using nano
void edit_zshrc() {
    const char* editor = getenv( "EDITOR" );
    if ( !editor ) editor = "nano";
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char cmd[2048];
    snprintf( cmd, sizeof( cmd ), "%s %s/.zshrc", editor, homeDir );
    system( cmd );
}

// Print the info in .zshrc file
void view_zshrc() {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char path[1024];
    snprintf( path, sizeof( path ), "%s/.zshrc", homeDir );
    print_file_contents( path );
}

int month_from_string( const char* str ) {
    const char* months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
    for ( int i = 0; i < 12; ++i ) {
        if ( strcasecmp( str, months[i] ) == 0 )
            return i + 1;
    }
    int num = atoi( str );
    if ( num >= 1 && num <= 12 ) return num;
    return 0;
}

// Repeat command n times
void repeat_command( int times, char** cmdArgs ) {
    char buffer[2048] = { 0 };
    for ( int i = 0; cmdArgs[i]; i++ ) {
        strcat( buffer, cmdArgs[i] );
        strcat( buffer, " " );
    }
    for ( int i = 0; i < times; i++ ) {
        system( buffer );
    }
}

// Get GitHub user info via curl
void gitstat( const char* username ) {
    printf( "GitHub Stats for %s\n", username );
    printf( "-------------------------\n" );
    char cmd[2048];
    snprintf( cmd, sizeof( cmd ),
        "curl -s https://api.github.com/users/%s | "
        "jq -r '.login, .name, \"Repos: \"+(.public_repos|tostring), "
        "\"Followers: \"+(.followers|tostring), "
        "\"Following: \"+(.following|tostring)'",
        username );
    system( cmd );
}

// Edit user script
void script_edit( const char* name ) {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char path[1024];
    snprintf( path, sizeof( path ), "%s/my scripts/%s.sh", homeDir, name );

    if ( access( path, F_OK ) == -1 ) {
        fprintf( stderr, "%s Script '%s.sh' not found in ~/my scripts\n", FLAG_ERR, name );
        return;
    }

    char cmd[2048];
    snprintf( cmd, sizeof( cmd ), "nano \"%s\"", path );
    system( cmd );
}

// Delete user script
void script_delete( const char* name ) {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char path[1024];
    snprintf( path, sizeof( path ), "%s/my scripts/%s.sh", homeDir, name );

    if ( unlink( path ) == 0 ) {
        printf( "Script '%s.sh' deleted from ~/my scripts\n", FLAG_OK, name );
    } else {
        fprintf( stderr, "%s Can't delete script", FLAG_ERR );
    }
}

// List all user scripts
void script_list() {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char path[1024];
    snprintf( path, sizeof( path ), "%s/my scripts", homeDir );

    DIR* dir = opendir( path );
    if ( !dir ) {
        fprintf( stderr, "%s Can't open ~/my scripts", FLAG_ERR );
        return;
    }

    struct dirent* entry;
    printf( "Your scripts:\n" );
    while (( entry = readdir( dir )) != NULL ) {
        if ( strstr( entry -> d_name, ".sh" )) {
            printf( " - %s\n", entry -> d_name );
        }
    }

    closedir( dir );
}

// Create user script
void script_create( const char* name, const char* scriptPath ) {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char targetDir[1024];
    snprintf( targetDir, sizeof( targetDir ), "%s/my scripts", homeDir );

    struct stat st = {0};
    if ( stat( targetDir, &st ) == -1 ) {
        mkdir( targetDir, 0700 );
    }

    char destPath[1024];
    snprintf( destPath, sizeof( destPath ), "%s/%s.sh", targetDir, name );

    FILE* src = fopen( scriptPath, "r" );
    if ( !src ) {
        fprintf( stderr, "%s Can't open source script", FLAG_ERR );
        return;
    }

    FILE* dest = fopen( destPath, "w" );
    if ( !dest ) {
        fprintf( stderr, "%s Can't create script", FLAG_ERR );
        fclose( src );
        return;
    }

    char buffer[1024];
    size_t bytes;
    while (( bytes = fread( buffer, 1, sizeof( buffer ), src )) > 0 ) {
        fwrite( buffer, 1, bytes, dest );
    }

    fclose( src );
    fclose( dest );
    chmod( destPath, 0755 );

    printf( "%s Script '%s.sh' created in ~/my scripts\n", FLAG_OK, RESET, name );
}

// Create empty user script using nano
void script_empty_create( const char* name ) {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char targetDir[1024];
    snprintf( targetDir, sizeof( targetDir ), "%s/my scripts", homeDir );

    struct stat st = {0};
    if ( stat( targetDir, &st ) == -1 ) {
        if ( mkdir( targetDir, 0700 ) == -1 ) {
            fprintf( stderr, "%s Can't create ~/my scripts directory", FLAG_ERR );
            return;
        }
    }

    char scriptPath[1024];
    snprintf( scriptPath, sizeof( scriptPath ), "%s/%s.sh", targetDir, name );

    if ( access( scriptPath, F_OK ) == 0 ) {
        fprintf( stderr, "%s Script '%s.sh' already exists in ~/my scripts\n", FLAG_ERR, name );
        return;
    }

    FILE* file = fopen( scriptPath, "w" );
    if ( !file ) {
        fprintf( stderr, "%s Can't create script file", FLAG_ERR );
        return;
    }
    fprintf( file, "#!/bin/bash\n\n" );
    fclose( file );

    if ( chmod( scriptPath, 0755 ) == -1 ) {
        fprintf( stderr, "%s Can't make script executable", FLAG_ERR );
        return;
    }

    printf( "%s Script '%s.sh' created in ~/my scripts\n", FLAG_OK, RESET, name );

    char cmd[2048];
    snprintf( cmd, sizeof( cmd ), "nano \"%s\"", scriptPath );
    system( cmd );
}


// Main Function
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
            "        (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n"
            "\n"
            "  Calendar commands:\n"
            "   cal - returns actual calendar month\n"
            "   Usage: cal [option] - return choosen calendar month\n"
            "     Options:\n"
            "      { jan, feb, mar, apr, may, jun,\n"
            "       jul, aug, sep, oct, nov, dec }\n"
            "      { 1, 2, 3, 4, 5, 6,\n"
            "       7, 8, 9, 10, 11, 12 }\n"
            "\n"
            "  Git commands:\n"
            "   gitstat [username] - github statistic of any user\n", FLAG_WARN );
        return 1;
    }

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
                    script_empty_create( argv[3] );
                } else {
                    fprintf( stderr, "%s Usage: script create [script name] [.sh file path]\n"
                    "     - сreates a command that executes your script\n"
                    "       (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n",
                    FLAG_WARN );
                }
            } else if ( strcmp( argv[2], "list" ) == 0 ) {
                script_list();
            } else {
                fprintf( stderr, "%s Usage: script create [script name] [.sh file path]\n"
                "     - сreates a command that executes your script\n"
                "       (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n",
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

    } else if ( strcmp( cmd, "cal" ) == 0 ) {
        int monthOverride = 0;
        if ( argc == 3 ) { // if programm got 3 arguments (return selected month)
            monthOverride = month_from_string( argv[2] );
            if ( monthOverride == 0 ) { 
                fprintf( stderr, "%s Invalid month: %s\n", FLAG_ERR, argv[2] );
                return 1;
            }
        }
        print_calendar( monthOverride );

    } else if ( strcmp( cmd, "gitstat" ) == 0 ) {
        if ( argc == 3 ) {
            gitstat( argv[2] );
        } else {
            fprintf( stderr, "%s Usage: gitstat [username]\n", FLAG_WARN );
        }

    } else if ( strcmp( cmd, "new" ) == 0 ) {
        restart_terminal_new_session();

    } else if ( strcmp( cmd, "newc" ) == 0 ) {
        restart_terminal_current_directory();

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
        fprintf( stderr, "%s Use:\n"
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
            "        (https://github.com/0netervezer0/Lupi-Zsh-Addons/blob/main/README.md for more)\n"
            "\n"
            "  Calendar commands:\n"
            "   cal - returns actual calendar month\n"
            "   Usage: cal [option] - return choosen calendar month\n"
            "     Options:\n"
            "      { jan, feb, mar, apr, may, jun,\n"
            "       jul, aug, sep, oct, nov, dec }\n"
            "      { 1, 2, 3, 4, 5, 6,\n"
            "       7, 8, 9, 10, 11, 12 }\n"
            "\n"
            "  Git commands:\n"
            "   gitstat [username] - github statistic of any user\n" );

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
