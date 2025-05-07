// #######################
// Lupi Zsh Addons V0.2.2
// by netervezer
// #######################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

const char* get_os_type() {
    #ifdef __APPLE__
        return "macOS";
    #else
        return "Linux";
    #endif
}

void print_calendar() {
    time_t t = time( NULL );
    struct tm *current_time = localtime( &t );
    
    int day = current_time->tm_mday;
    int month = current_time->tm_mon + 1;    
    int year = current_time->tm_year + 1900; 

    struct tm first_day = *current_time;
    first_day.tm_mday = 1;
    mktime( &first_day );

    int start_day = first_day.tm_wday;

    int days_in_month;
    if ( month == 2 ) {
        if (( year % 400 == 0) || ( year % 100 != 0 && year % 4 == 0 )) {
            days_in_month = 29;
        } else {
            days_in_month = 28;
        }
    } else if ( month == 4 || month == 6 || month == 9 || month == 11 ) {
        days_in_month = 30;
    } else {
        days_in_month = 31;
    }

    const char *month_names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    printf( "\n    %s %d\n", month_names[month-1], year );
    printf(  "Su Mo Tu We Th Fr Sa\n" );

    for ( int i = 0; i < start_day; i++ ) {
        printf( "   " );
    }

    for ( int d = 1; d <= days_in_month; d++ ) {
        if ( d == day ) {
            printf( "\033[1;31m%2d\033[0m ", d );
        } else {
            printf( "%2d ", d );
        }

        if (( start_day + d ) % 7 == 0 || d == days_in_month ) {
            printf( "\n" );
        }
    }
    printf( "\n" );
}

void restart_terminal_new_session() {
    const char* os_type = get_os_type();

    if ( strcmp( os_type, "macOS" ) == 0) {
        // macOS
        system( "open -a Terminal ~" );
    } else {
        // Linux (GNOME Terminal)
        system( "gnome-terminal -- bash -c 'exec bash'" );
    }

    printf( "New session started...\n" );
}

void restart_terminal_current_directory() {
    const char* os_type = get_os_type();
    char cwd[1024];

    if ( getcwd( cwd, sizeof( cwd )) == NULL ) {
        perror( "Can't get current directory" );
        return;
    }

    if ( strcmp( os_type, "macOS" ) == 0 ) {
        // macOS
        char command[2048];
        snprintf( command, sizeof( command ), "open -a Terminal \"%s\"", cwd );
        system( command );
    } else {
        // Linux (GNOME Terminal)
        char command[2048];
        snprintf( command, sizeof( command ), "gnome-terminal -- bash -c 'cd %s; exec bash'", cwd );
        system( command );
    }

    printf( "New session started in current directory\n" );
}

long calculate_directory_size( const char* path ) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    long total_size = 0;

    if (( dir = opendir( path )) == NULL ) {
        perror( "Can't find path" );
        return -1;
    }

    while (( entry = readdir( dir )) != NULL ) {
        if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 ) {
            continue;
        }

        char full_path[1024];
        snprintf( full_path, sizeof( full_path ), "%s/%s", path, entry->d_name );

        if ( stat( full_path, &statbuf ) == -1 ) {
            perror( "Can't get file information" );
            continue;
        }

        if ( S_ISDIR( statbuf.st_mode )) {
            long dir_size = calculate_directory_size( full_path );
            if ( dir_size == -1 ) {
                closedir( dir );
                return -1;
            }
            total_size += dir_size;
        } else {
            total_size += statbuf.st_size;
        }
    }

    closedir( dir );
    return total_size;
}

void clear_directory( const char* path ) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;

    if (( dir = opendir( path )) == NULL ) {
        perror( "Can't open folder" );
        return;
    }

    while (( entry = readdir(dir)) != NULL ) {
        if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 ) {
            continue;
        }

        char full_path[1024];
        snprintf( full_path, sizeof( full_path ), "%s/%s", path, entry->d_name );

        if ( stat( full_path, &statbuf ) == -1 ) {
            perror( "Can't get file include" );
            continue;
        }

        if ( S_ISDIR( statbuf.st_mode )) {
            clear_directory( full_path );
            rmdir( full_path ); 
        } else {
            if ( unlink( full_path) == -1 ) {
                perror( "Can't delete files" );
            }
        }
    }
}

void clear_zsh_history( const char* home_dir ) {
    char zsh_history_path[1024];
    snprintf( zsh_history_path, sizeof( zsh_history_path ), "%s/.zsh_history", home_dir );

    FILE* zsh_history_file = fopen( zsh_history_path, "w" );
    if ( zsh_history_file != NULL ) {
        fclose( zsh_history_file );
    } else {
        perror( "Can't clear .zsh_history" );
    }
}

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


int main( int argc, char* argv[] ) {

    if ( argc != 2) {
        fprintf( stderr, "Lupi Zsh Addons v0.2.3\nUse:\n"
                    "  help - to see the command list\n"
                    "  cache - to see the terminal cache size and clean it\n"
                    "  hist - to see the command history of your terminal\n"
                    "  new - start new terminal session\n"
                    "  newc - start new terminal session in current directory\n"
                    "  cal - returns actual date and calendar\n" );
        return 1;
    }

    if ( strcmp( argv[1], "help" ) == 0 ) {
        fprintf( stderr, "Use:\n"
                    "  help - to see the command list\n"
                    "  cache - to see the terminal cache size and clean it\n"
                    "  hist - to see the command history of your terminal\n"
                    "  new - start new terminal session\n"
                    "  newc - start new terminal session in current directory\n"
                    "  cal - returns actual date and calendar\n" );
    }

    else if ( strcmp( argv[1], "cal" ) == 0 ) {
        print_calendar();

    } else if ( strcmp( argv[1], "cache" ) == 0 ) {
        const char* home_dir = getenv( "HOME" );
        if ( home_dir == NULL ) {
            fprintf( stderr, "Can't find HOME user path\n" );
            return 1;
        }

        char sessions_dir[1024];
        snprintf( sessions_dir, sizeof( sessions_dir ), "%s/.zsh_sessions", home_dir );

        struct stat statbuf;
        if ( stat( sessions_dir, &statbuf ) == -1 || !S_ISDIR( statbuf.st_mode )) {
            fprintf( stderr, "Can't find .zsh_sessions folder\n" );
            return 1;
        }

        long dir_size = calculate_directory_size( sessions_dir );
        if ( dir_size == -1 ) {
            fprintf( stderr, "Can't calculate size of .zsh_sessions\n" );
            return 1;
        }

        char zsh_history_path[1024];
        snprintf( zsh_history_path, sizeof( zsh_history_path ), "%s/.zsh_history", home_dir );

        long zsh_history_size = 0;
        if ( stat( zsh_history_path, &statbuf ) != -1 ) {
            zsh_history_size = statbuf.st_size;
        }

        long total_cache_size = dir_size + zsh_history_size;
        printf( "Cache size: %ld bytes\n", total_cache_size );

        printf( "Clean cache? (y/n): " );
        char response;
        scanf( " %c", &response );

        if ( response == 'y' || response == 'Y' ) {
            clear_directory( sessions_dir );
            clear_zsh_history( home_dir );
            printf( "Cache cleaned\n" );
        } else {
            printf( "Clean cancelled\n" );
        }

    } else if ( strcmp( argv[1], "newc" ) == 0) {
        restart_terminal_current_directory();
        

    } else if ( strcmp( argv[1], "new" ) == 0 ) {
        restart_terminal_new_session();
        

    } else if ( strcmp( argv[1], "hist" ) == 0 ) {
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

    } else {
        fprintf( stderr, "Unknown argument '%s'. Use 'help' to see the command list\n", argv[1] );
        return 1;
    }

    return 0;
}
