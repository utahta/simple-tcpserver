/**
 * easy test client.
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
 
#define CONNECT_ADDR "127.0.0.1"
#define CONNECT_PORT (12345)
 
bool setnonblock(int fd)
{
    int flags = fcntl( fd, F_GETFL );
    if( flags < 0 ){
        return false;
    }
 
    if( fcntl( fd, F_SETFL, flags |= O_NONBLOCK ) < 0 ){
        return false;
    }
    return true;
}
 
int main(void)
{
    int ret = 0, sock = 0, clilen = 0;
    struct sockaddr_in cliaddr;
 
    // socket
    if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ){
        puts( "socket error!!" );
        return 1;
    }
 
    memset( &cliaddr, 0, sizeof( cliaddr ) );
    cliaddr.sin_addr.s_addr = inet_addr( CONNECT_ADDR );
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons( CONNECT_PORT );
 
    clilen = sizeof( cliaddr );
    if( ( ret = connect( sock, (struct sockaddr *)&cliaddr, clilen ) ) < 0 ){
        perror( "connect()" );
        return 1;
    }
    if( !setnonblock( sock ) ){
        perror( "setnonblock()" );
    }
 
    char str[1024];
    char *strpos;
    while( 1 )
    {
        printf( "Enter your text:" );
        memset( str, 0, sizeof( str ) );
        fgets( str, sizeof(str), stdin );
 
        // \n を除去
        if( ( strpos = strchr( str, '\n' ) ) != NULL ){
            (*strpos) = '\0';
        }
 
        // exit
        if( strncmp( str, "exit", 4 ) == 0 )
            break;
 
        // send
        char *p = str;
        int len = strlen( str );
        int res = 0;
        while( len > 0 ){
            do{
                res = send( sock, str, len, 0 );
                if( errno != EINTR ){
                    break;
                }
            }while( 1 );

            if( errno < 0 ){
                perror( "send()" );
                break;
            }
            p += res;
            len -= res;
        }
    }
 
    // quit.
    while( 1 ){
        ret = recv( sock, str, sizeof(str), 0 );
        if( ret <= 0 )
            break;
    }
    close( sock );
 
    return 0;
}

