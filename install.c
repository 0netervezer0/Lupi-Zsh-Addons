// Installation tool for Lupi Zsh Addons

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf( "Starting installation...\n" );
    if ( geteuid() == 0 ) {
        printf( "Administrator rights recieved...\n" );
        system( "chmod u+x lupi" );
        system( "sudo mv lupi /usr/local/bin/" );
    } else {
        fprintf( stderr, "Administrator rights not received!\n" );
        printf( "Use: sudo ./install" );

        return 1;
    }
    return 0;
}