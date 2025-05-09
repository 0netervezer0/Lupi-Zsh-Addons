// Lupi Zsh Addons V0.3.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>


// Get OS type (macOS/Linuz)
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
        perror( "Can't get current directory" );
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
        perror( "Can't find path" );
        return -1;
    }

    while (( entry = readdir( dir )) != NULL ) {
        if ( strcmp( entry -> d_name, "." ) == 0 || strcmp( entry -> d_name, ".." ) == 0 ) {
            continue;
        }

        char fullPath[1024];
        snprintf( fullPath, sizeof( fullPath ), "%s/%s", path, entry -> d_name );

        if ( stat( fullPath, &statbuf ) == -1 ) {
            perror( "Can't get file information" );
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
        perror( "Can't open folder" );
        return;
    }

    while (( entry = readdir(dir)) != NULL ) {
        if ( strcmp( entry -> d_name, "." ) == 0 || strcmp( entry -> d_name, ".." ) == 0 ) {
            continue;
        }

        char fullPath[1024];
        snprintf( fullPath, sizeof( fullPath ), "%s/%s", path, entry -> d_name );

        if ( stat( fullPath, &statbuf ) == -1 ) {
            perror( "Can't get file include" );
            continue;
        }

        if ( S_ISDIR( statbuf.st_mode )) {
            clear_directory( fullPath );
            rmdir( fullPath ); 
        } else {
            if ( unlink( fullPath) == -1 ) {
                perror( "Can't delete files" );
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
        perror( "Can't clear .zsh_history" );
    }
}

// Print content in file 
void print_file_contents( const char* filepath ) {
    FILE* file = fopen( filepath, "r" );
    if ( file == NULL ) {
        perror( "Can't open file" );
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
        fprintf( stderr, "HOME not set\n" );
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
        fprintf( stderr, "HOME not set\n" );
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

// Disable or enable history
void sethist( const char* mode ) {
    const char* home = getenv( "HOME" );
    char histfile[1024];
    snprintf( histfile, sizeof( histfile ), "%s/.zshrc", home );

    FILE* file = fopen( histfile, "a" );
    if ( file == NULL ) {
        perror( "Can't open .zshrc" );
        return;
    }

    if ( strcmp( mode, "off" ) == 0 ) {
        fprintf( file, "\nsetopt NO_HIST_IGNORE_ALL\nsetopt NO_HIST_SAVE_NO_DUPS\nunsetopt HISTFILE\n" );
        printf( "History disabled\n" );
    } else if ( strcmp( mode, "on" ) == 0 ) {
        fprintf( file, "\nsetopt HISTFILE\n" );
        printf( "History enabled\n" );
    } else {
        printf( "Unknown sechist option: %s\n", mode );
    }

    fclose( file );
}

// Get GitHub user info via curl
void gitstat( const char* username ) {
    char cmd[1024];
    snprintf( cmd, sizeof( cmd ),
      "curl -s https://api.github.com/users/%s | grep -E \"login|name|public_repos|followers|following\"",
      username );
    system( cmd );
}

// Main Function
int main( int argc, char* argv[] ) {
    if ( argc < 2 ) {
        fprintf( stderr, "Lupi Zsh Addons v0.3.1\nUse:\n"
            "  Default commands:\n"
            "   help - the command list\n"
            "   cache - the terminal cache size and clean it\n"
            "   new - start new terminal session\n"
            "   newc - start new terminal session in current directory\n"
            "   space - show disk usage of home directory\n"
            "\n"
            "  Resource commands:\n"
            "   hist - the command history of your terminal\n"
            "   Usage: sethist [option] - turn on/off command history\n"
            "     Options:\n"
            "      on - turn on\n"
            "      off - turn off\n"
            "   Usage: rc [option] - show or edit .zshrc\n"
            "     Options:\n"
            "      view - show .zshrc contents\n"
            "      edit - edit .zshrc using nano\n"
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
        return 1;
    }

    const char* cmd = argv[1];

    if ( strcmp( cmd, "space" ) == 0 ) {
        show_disk_space();

    } else if ( strcmp( cmd, "sethist" ) == 0 ) {
        if ( argc == 3 ) {
            sethist( argv[2] );
        } else {
            fprintf( stderr, "Usage: sethist [option] - turn on/off command history\n"
            "  Options:\n"
            "   on - turn on\n"
            "   off - turn off\n" );
        }

    } else if ( strcmp( cmd, "rc" ) == 0 ) {
        if ( argc == 3 ) {
            if ( strcmp( argv[2], "view" ) == 0 ) {
                view_zshrc();
            } else if ( strcmp( argv[2], "edit" ) == 0 ) {
                edit_zshrc();
            } else {
                fprintf( stderr, "Usage: lupi rc [option] \n"
                "  Options:\n"
                "   view - show .zshrc contents\n"
                "   edit - edit .zshrc using nano\n" );
            }
        } else {
            fprintf( stderr, "Usage: lupi rc [option] \n"
                "  Options:\n"
                "   view - show .zshrc contents\n"
                "   edit - edit .zshrc using nano\n" );
        }

    } else if ( strcmp( cmd, "cal" ) == 0 ) {
        int monthOverride = 0;
        if ( argc == 3 ) { // if programm got 3 arguments (return selected month)
            monthOverride = month_from_string( argv[2] );
            if ( monthOverride == 0 ) { 
                fprintf( stderr, "Invalid month: %s\n", argv[2] );
                return 1;
            }
        }
        print_calendar( monthOverride );

    } else if ( strcmp( cmd, "gitstat" ) == 0 ) {
        if ( argc == 3 ) {
            gitstat( argv[2] );
        } else {
            fprintf( stderr, "Usage: gitstat [username]\n" );
        }

    } else if ( strcmp( cmd, "new" ) == 0 ) {
        restart_terminal_new_session();

    } else if ( strcmp( cmd, "newc" ) == 0 ) {
        restart_terminal_current_directory();

    } else if ( strcmp( cmd, "hist" ) == 0 ) {
        const char* home_dir = getenv( "HOME" );
        if ( home_dir == NULL ) {
            fprintf( stderr, "Can't find HOME user path\n" );
            return 1;
        }

        char filepath[1024];
        snprintf( filepath, sizeof( filepath ), "%s/.zsh_history", home_dir );

        FILE* file = fopen( filepath, "r" );
        if ( file == NULL ) {
            perror( "Can't find .zsh_history file" );
            return 1;
        }
        fclose( file );

        printf( "Your command history: \n" );
        print_file_contents( filepath );

    } else if ( strcmp( cmd, "cache" ) == 0 ) {
        const char* homeDir = getenv( "HOME" );
        if ( homeDir == NULL ) {
            fprintf( stderr, "Can't find HOME user path\n" );
            return 1;
        }

        char sessionsDir[1024];
        snprintf( sessionsDir, sizeof( sessionsDir ), "%s/.zsh_sessions", homeDir );

        struct stat statbuf;
        if ( stat( sessionsDir, &statbuf ) == -1 || !S_ISDIR( statbuf.st_mode )) {
            fprintf( stderr, "Can't find .zsh_sessions folder\n" );
            return 1;
        }

        long dirSize = calculate_directory_size( sessionsDir );
        if ( dirSize == -1 ) {
            fprintf( stderr, "Can't calculate size of .zsh_sessions\n" );
            return 1;
        }

        char zshHistoryPath[1024];
        snprintf( zshHistoryPath, sizeof( zshHistoryPath ), "%s/.zsh_history", homeDir );

        long zshHistorySize = 0;
        if ( stat( zshHistoryPath, &statbuf ) != -1 ) {
            zshHistorySize = statbuf.st_size;
        }

        long totalCacheSize = dirSize + zshHistorySize;
        printf( "Cache size: %ld bytes\n", totalCacheSize );

        printf( "Clean cache? (y/n): " );
        char response;
        scanf( " %c", &response );

        if ( response == 'y' || response == 'Y' ) {
            clear_directory( sessionsDir );
            clear_zsh_history( homeDir );
            printf( "Cache cleaned\n" );
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
            "  Resource commands:\n"
            "   hist - the command history of your terminal\n"
            "   Usage: sethist [option] - turn on/off command history\n"
            "     Options:\n"
            "      on - turn on\n"
            "      off - turn off\n"
            "   Usage: rc [option] - show or edit .zshrc\n"
            "     Options:\n"
            "      view - show .zshrc contents\n"
            "      edit - edit .zshrc using nano\n"
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
        fprintf( stderr, "Unknown argument '%s'. Use 'help' to see the command list\n", cmd );
        return 1;
    }

    return 0;
}
