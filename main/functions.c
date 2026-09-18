// Lupi Zsh Addons v1.1
// * Function Description File * //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

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

int get_os_type() {
    #ifdef __APPLE__
        return 9;
    #elif __linux__
        return 8;
    #else
        return 0;
    #endif
}

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

static void write_script_template( const char* path ) {
    FILE* file = fopen( path, "w" );
    if ( file == NULL ) {
        fprintf( stderr, "%s Can't create script template\n", FLAG_ERR );
        return;
    }

    fprintf( file,
        "#!/bin/bash\n\n"
        "# Lupi script template\n"
        "# Usage: lupi <script-name> <arg0> <arg1> ...\n"
        "# Example: lupi myscript $arg0$ $arg1$\n"
        "# Replace $arg0$, $arg1$, ... with the values you pass to lupi.\n"
        "# Example command: python3 blackbird --username $arg0$\n\n"
    );

    fclose( file );
    chmod( path, 0755 );
}

static int get_script_placeholder_index( const char* text, const char** next ) {
    if ( strncmp( text, "$arg", 4 ) != 0 ) {
        return -1;
    }

    const char* digits = text + 4;
    if ( *digits == '\0' || !isdigit( (unsigned char)*digits ) ) {
        return -1;
    }

    const char* cursor = digits;
    while ( *cursor != '\0' && isdigit( (unsigned char)*cursor ) ) {
        cursor++;
    }

    if ( *cursor != '$' ) {
        return -1;
    }

    if ( next != NULL ) {
        *next = cursor + 1;
    }

    return atoi( digits );
}

static int should_ignore_placeholder( const char* placeholderPos, const char* content ) {
    const char* lineStart = placeholderPos;
    while ( lineStart > content && lineStart[-1] != '\n' && lineStart[-1] != '\r' ) {
        lineStart--;
    }

    const char* cursor = lineStart;
    while ( *cursor == ' ' || *cursor == '\t' ) {
        cursor++;
    }

    return *cursor == '#';
}

static int count_script_placeholders( const char* content ) {
    int expected = 0;
    const char* cursor = content;

    while ( *cursor != '\0' ) {
        const char* next = NULL;
        int index = get_script_placeholder_index( cursor, &next );
        if ( index >= 0 ) {
            if ( !should_ignore_placeholder( cursor, content ) ) {
                if ( index + 1 > expected ) {
                    expected = index + 1;
                }
            }
            cursor = next;
        } else {
            cursor++;
        }
    }

    return expected;
}

static char* shell_escape_argument( const char* value ) {
    size_t length = strlen( value );
    char* escaped = malloc( length * 4 + 3 );
    if ( escaped == NULL ) {
        return NULL;
    }

    size_t pos = 0;
    escaped[pos++] = '\'';

    for ( size_t i = 0; i < length; ++i ) {
        if ( value[i] == '\'' ) {
            escaped[pos++] = '\'';
            escaped[pos++] = '\\';
            escaped[pos++] = '\'';
            escaped[pos++] = '\'';
        } else {
            escaped[pos++] = value[i];
        }
    }

    escaped[pos++] = '\'';
    escaped[pos] = '\0';
    return escaped;
}

int execute_script_with_arguments( const char* scriptPath, int argCount, char* const args[] ) {
    FILE* file = fopen( scriptPath, "r" );
    if ( file == NULL ) {
        fprintf( stderr, "%s Can't open script '%s'\n", FLAG_ERR, scriptPath );
        return 1;
    }

    fseek( file, 0, SEEK_END );
    long fileSize = ftell( file );
    if ( fileSize < 0 ) {
        fclose( file );
        fprintf( stderr, "%s Can't read script '%s'\n", FLAG_ERR, scriptPath );
        return 1;
    }
    rewind( file );

    char* content = malloc( (size_t)fileSize + 1 );
    if ( content == NULL ) {
        fclose( file );
        fprintf( stderr, "%s Can't allocate memory for script '%s'\n", FLAG_ERR, scriptPath );
        return 1;
    }

    size_t readCount = fread( content, 1, (size_t)fileSize, file );
    fclose( file );
    content[readCount] = '\0';

    int expectedArgs = count_script_placeholders( content );
    if ( argCount != expectedArgs ) {
        fprintf( stderr, "%s Script '%s' expects %d argument(s), but %d were passed\n",
            FLAG_ERR, scriptPath, expectedArgs, argCount );
        free( content );
        return 1;
    }

    char* processed = malloc( strlen( content ) * 4 + 1 );
    if ( processed == NULL ) {
        free( content );
        fprintf( stderr, "%s Can't allocate memory for script processing\n", FLAG_ERR );
        return 1;
    }

    size_t processedLen = 0;
    const char* cursor = content;
    while ( *cursor != '\0' ) {
        const char* next = NULL;
        int index = get_script_placeholder_index( cursor, &next );
        if ( index >= 0 ) {
            if ( should_ignore_placeholder( cursor, content ) ) {
                while ( cursor < next ) {
                    processed[processedLen++] = *cursor;
                    cursor++;
                }
                continue;
            }

            if ( index >= argCount ) {
                fprintf( stderr, "%s Placeholder $arg%d$ used but no argument was provided\n", FLAG_ERR, index );
                free( processed );
                free( content );
                return 1;
            }

            char* escaped = shell_escape_argument( args[index] );
            if ( escaped == NULL ) {
                free( processed );
                free( content );
                fprintf( stderr, "%s Can't prepare script arguments\n", FLAG_ERR );
                return 1;
            }

            size_t escapedLen = strlen( escaped );
            memcpy( processed + processedLen, escaped, escapedLen );
            processedLen += escapedLen;
            free( escaped );
            cursor = next;
        } else {
            processed[processedLen++] = *cursor;
            cursor++;
        }
    }
    processed[processedLen] = '\0';
    free( content );

    char tempPath[] = "/tmp/lupi-script-XXXXXX.sh";
    int fd = mkstemp( tempPath );
    if ( fd == -1 ) {
        free( processed );
        fprintf( stderr, "%s Can't create temporary script\n", FLAG_ERR );
        return 1;
    }

    FILE* tempFile = fdopen( fd, "w" );
    if ( tempFile == NULL ) {
        close( fd );
        unlink( tempPath );
        free( processed );
        fprintf( stderr, "%s Can't write temporary script\n", FLAG_ERR );
        return 1;
    }

    fwrite( processed, 1, processedLen, tempFile );
    fclose( tempFile );
    chmod( tempPath, 0700 );

    char command[4096];
    snprintf( command, sizeof( command ), "bash \"%s\"", tempPath );
    int exitCode = system( command );
    unlink( tempPath );
    free( processed );

    return exitCode;
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

    printf( "%s Script '%s.sh' created in ~/my scripts\n", FLAG_OK, name );
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

    write_script_template( scriptPath );

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

    FILE* file = fopen( path, "r" );
    if ( file != NULL ) {
        fseek( file, 0, SEEK_END );
        long fileSize = ftell( file );
        fclose( file );

        if ( fileSize == 0 ) {
            write_script_template( path );
        }
    }

    char cmd[2048];
    snprintf( cmd, sizeof( cmd ), "nano \"%s\"", path );
    system( cmd );
}

// Remove user script
void script_remove( const char* name ) {
    const char* homeDir = getenv( "HOME" );
    if ( !homeDir ) {
        fprintf( stderr, "%s HOME not set\n", FLAG_ERR );
        return;
    }

    char path[1024];
    snprintf( path, sizeof( path ), "%s/my scripts/%s.sh", homeDir, name );

    if ( unlink( path ) == 0 ) {
        printf( "%s Script '%s.sh' removed from ~/my scripts\n", FLAG_OK, name );
    } else {
        fprintf( stderr, "%s Can't remove script\n", FLAG_ERR );
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
