/*
 * tw-mailer basic
 *
 * SERVER
 */
#include <unistd.h>
#include <netinet/in.h>
#include <csignal>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <vector>

#include "../header/server_utils.h"

/* *************************************************** */

namespace twMailer
{
    using std::cerr;
    using std::cout;
    using std::endl;
    using std::string;
    using std::filesystem::create_directory;
}

using namespace twMailer;

bool abortRequested = false;
int create_socket = -1;         //
int new_socket = -1;            // hold the file descriptor
string spool;                   // spool directory

int main(int argc, char* argv[])
{
    int port;
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;

    if(argc < 3)
    {
        cerr << "error: no port or Mail-spool-directoryname passed" << endl;
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);       //converts char* to int
    spool = argv[2];                //saves spool directory name
    create_directory(spool);

    ////////////////////////////////////////////////////////////////////////////
    // SIGNAL HANDLER
    // SIGINT (Interrup: ctrl+c) is registered in for handling
    // https://man7.org/linux/man-pages/man2/signal.2.html
    if( signal(SIGINT, signalHandler) == SIG_ERR)
    {
        cerr << "error: signal can not be registered" << endl;
        return EXIT_FAILURE;

    }

    ////////////////////////////////////////////////////////////////////////////
    // CREATE A SOCKET
    // https://man7.org/linux/man-pages/man2/socket.2.html
    // https://man7.org/linux/man-pages/man7/ip.7.html
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    // IPv4, TCP (connection oriented), IP (same as client)
    if((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        cerr << "Socket error" << endl;
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // SET SOCKET OPTIONS
    // https://man7.org/linux/man-pages/man2/setsockopt.2.html
    // https://man7.org/linux/man-pages/man7/socket.7.html
    // socket, level, optname, optvalue, optlen
    if(setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
    {
        cerr << "set socket options - reuseAddr" << endl;
        return EXIT_FAILURE;
    }

    if(setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEPORT,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
    {
        cerr << "set socket options - reusePort" << endl;
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // INIT ADDRESS
    // Attention: network byte order => big endian
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    ////////////////////////////////////////////////////////////////////////////
    // ASSIGN AN ADDRESS WITH PORT TO SOCKET
    if( bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1 )
    {
        cerr << "bind error" << endl;
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // ALLOW CONNECTION ESTABLISHING
    // Socket, Backlog (= count of waiting connections allowed)
    if( listen(create_socket, 5) == -1)
    {
        cerr << "listen error" << endl;
        return EXIT_FAILURE;
    }

    while(!abortRequested)
    {
        cout << "Waiting for connections..." << endl;

        /////////////////////////////////////////////////////////////////////////
        // ACCEPTS CONNECTION SETUP
        // blocking, might have an accept-error on ctrl+c
        addrlen = sizeof(struct sockaddr_in);
        if ( (new_socket = accept(create_socket,(struct sockaddr *)&cliaddress, &addrlen)) == -1 )
        {
            if(abortRequested)
            {
                cerr << "accept error after aborted" << endl;
            }
            else
            {
                cerr << "accept error" << endl;
            }
            break;
        }

        /////////////////////////////////////////////////////////////////////////
        // START CLIENT
        // ignore printf error handling
        cout << "Client connected from "
             << inet_ntoa(cliaddress.sin_addr)
             << ": " << ntohs(cliaddress.sin_port)
             << endl;

        clientCommunication(&new_socket);

        new_socket = -1;
    }

    // frees the descriptor
    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown create_socket");
        }
        if (close(create_socket) == -1)
        {
            perror("close create_socket");
        }
        create_socket = -1;
    }

    return EXIT_SUCCESS;
}