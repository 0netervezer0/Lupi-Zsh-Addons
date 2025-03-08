#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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
            perror( "Can't get .zsh_history information" );
            continue;
        }

        if ( S_ISDIR(statbuf.st_mode )) {
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

    closedir( dir );
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
        fprintf( stderr, "Lupi Zsh Addons v0.1\nUse:\n"
                    "  help - to see the command list\n"
                    "  cache - to see the cache size and clean it\n"
                    "  about - to see the information about your zsh\n"
                    "  history - to see the command history of your terminal\n" );
        return 1;
    }

    if ( strcmp( argv[1], "help" ) == 0 ) {
        fprintf( stderr, "Lupi Zsh Addons v0.1\nUse:\n"
                    "  help - to see the command list\n"
                    "  cache - to see the cache size and clean it\n"
                    "  about - to see the information about your zsh\n"
                    "  history - to see the command history of your terminal\n" );

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
            fprintf( stderr, "Can't find folder\n" );
            return 1;
        }

        long dir_size = calculate_directory_size( sessions_dir );
        if ( dir_size == -1 ) {
            fprintf( stderr, "Can't calculate size\n" );
            return 1;
        }

        printf( "Cache size: %ld byte\n", dir_size );

        printf( "Clean cache? (y/n): " );
        char response;
        scanf( " %c", &response );

        if ( response == 'y' || response == 'Y' ) {
            clear_directory( sessions_dir );
            printf( "Cache cleaned\n" );
        } else {
            printf( "Clean cancelled\n" );
        }

    } else if ( strcmp( argv[1], "about" ) == 0 ) {
        printf( "###\n" );

    } else if ( strcmp( argv[1], "history" ) == 0 ) {
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
