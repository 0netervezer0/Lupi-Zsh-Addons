// Lupi Zsh Addons V0.5
// * Function Description File * //
// TODO: Fix new/newc commands

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

// ! ----------------------------------------------

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

// ! ----------------------------------------------

// Open new terminal session
void start_terminal_session() {
    system( "open -a Terminal ~" );

    printf( "New session started\n" );
}

// Open new terminal session in current directory
void start_terminal_session_current_directory() {
    char cwd[1024];

    if ( getcwd( cwd, sizeof( cwd )) == NULL ) {
        fprintf( stderr, "%s Can't get current directory", FLAG_ERR );
        return;
    }

    char command[2048];
    snprintf( command, sizeof( command ), "open -a Terminal \"%s\"", cwd );
    system( command );

    printf( "New session started in current directory\n" );
}

// ! ----------------------------------------------

// Disk space (df -h ~)
void show_disk_space() {
    system( "df -h ~" );
}

// ! ----------------------------------------------

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

// ! ----------------------------------------------

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

// ! ----------------------------------------------

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
        fprintf( stderr, "%s Can't open source script\n", FLAG_ERR );
        return;
    }

    FILE* dest = fopen( destPath, "w" );
    if ( !dest ) {
        fprintf( stderr, "%s Can't create script\n", FLAG_ERR );
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
void script_create_empty( const char* name ) {
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
            fprintf( stderr, "%s Can't create ~/my scripts\n", FLAG_ERR );
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
        fprintf( stderr, "%s Can't create script file\n", FLAG_ERR );
        return;
    }
    fprintf( file, "#!/bin/bash\n\n" );
    fclose( file );

    if ( chmod( scriptPath, 0755 ) == -1 ) {
        fprintf( stderr, "%s Can't make script executable\n", FLAG_ERR );
        return;
    }

    printf( "%s Script '%s.sh' created in ~/my scripts\n", FLAG_OK,  name );

    char cmd[2048];
    snprintf( cmd, sizeof( cmd ), "nano \"%s\"", scriptPath );
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
        printf( "%s Script '%s.sh' deleted from ~/my scripts\n", FLAG_OK, name );
    } else {
        fprintf( stderr, "%s Can't delete script\n", FLAG_ERR );
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

// ! ----------------------------------------------