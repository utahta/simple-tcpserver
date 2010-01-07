/**
 * easy tcp server.
 */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISTEN_PORT (12345)

static fd_set g_fds; ///< manage socket.

static void client_proc( int sd )
{
    char buff[1024];
    memset( buff, 0, sizeof( buff ) );

    int res = recv( sd, buff, sizeof( buff ), MSG_DONTWAIT );
    if( res < 0 ){
        if( ( errno == EAGAIN ) ||
            ( errno == EWOULDBLOCK ) ){
            printf( "Not ready yet.\n" );
        }
        else{
            perror( "recv()" );
            FD_CLR( sd, &g_fds );
            close( sd );
            return;
        }
    }
    else if( res == 0 ){
        printf( "disconeccted. sd:%d\n", sd );
        FD_CLR( sd, &g_fds );
        close( sd );
        return;
    }

    printf( "received. sd:%d buff:%s\n", sd, buff );
}

int main()
{
    int listen_sd = socket( PF_INET, SOCK_STREAM, 0 );
    if( listen_sd < 0 ) {
        perror( "socket()" );
        exit( 1 );
    }

    struct sockaddr_in sock_in;
    bzero( &sock_in, sizeof( sock_in ) );
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons( LISTEN_PORT );
    sock_in.sin_addr.s_addr = INADDR_ANY;
    
    int is_reuseaddr = 1;
    setsockopt( listen_sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&is_reuseaddr, sizeof( is_reuseaddr ) );

    if( bind( listen_sd, (struct sockaddr *) &sock_in, sizeof( sock_in ) ) < 0 ){
        perror( "bind()" );
        exit( 1 );
    }

    if( listen( listen_sd, 128 ) < 0 ){
        perror( "listen" );
        exit( 1 );
    }

    FD_ZERO( &g_fds );
    FD_SET( listen_sd, &g_fds );
    int maxsd = listen_sd;
    
   
    fd_set rfds;
    int num_ready = 0;
    // wait for client requests.
    for(;;){
        rfds = g_fds;
        
        // wait for event.
        num_ready = select( maxsd+1, &rfds, NULL, NULL, NULL );
        if( num_ready < 0 ){
            continue;
        }
        
        int sd;
        for( sd = 0; sd < maxsd+1; ++sd ){
            if( FD_ISSET( sd, &rfds ) ){
                if( sd == listen_sd ){
                    // accept a client.
                    struct sockaddr_in cs_in;
                    socklen_t len = sizeof( cs_in );
                    
                    int client_sd = accept( listen_sd, (struct sockaddr *) &cs_in, &len );
                    if( client_sd < 0 ){
                        perror( "accept()" );
                    }
                    else{
                        FD_SET( client_sd, &g_fds ); 
                        if( maxsd < client_sd ){
                            maxsd = client_sd;
                        }
                        printf( "connected. sd:%d\n", client_sd );
                    }
                }
                else{
                    // client request.
                    client_proc( sd );
                }
            }
        }
    }
    
    return 0;
}

