// Lupi Zsh Addons V0.4.2
// * Function Header File * //

#ifndef FUNCTIONS_H
#define FUNCTIONS_H


// ! "Default commands"
// * Functions for "cache" command
long calculate_directory_size( const char* path );
void clear_directory( const char* path );
void clear_zsh_history( const char* homeDir );

// * Functions for "new"/"newc" command
void start_terminal_session();
void start_terminal_session_current_directory();

// * Functions for "space" command
void show_disk_space();

// ! "Environment commands"
// * Functions for "hist" command
void print_file_contents( const char* filepath );

// * Functions for "rc" command
void view_zshrc();
void edit_zshrc();

// * Functions for "script" command
void script_create( const char* name, const char* scriptPath );
void script_create_empty( const char* name );
void script_edit( const char* name );
void script_delete( const char* name );
void script_list();


#endif