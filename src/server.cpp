/*
 * tw-mailer basic
 *
 * SERVER
 */
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <csignal>
#include <cstring>
#include <arpa/inet.h>
#include <string>

/* *************************************************** */

#define BUF 1024

using std::cerr;
using std::cout;
using std::endl;
using std::string;

bool abortRequested = false;
int create_socket = -1;         //
int new_socket = -1;            // hold the file descriptor

void clientCommunication(void* data);
void signalHandler(int sig);
void print_usage(char* program_name);
bool send_client(const int* socket, string &buffer);   //sends client a string
bool receive_client(const int *socket, char *buffer, string &message);
bool handle(int *socket, char *buffer, string &message);

void OK(int *socket, string &message);
void ERR(int *socket, string &message);

//// PROTOCOLS
bool send_protocol(int *socket, char *buffer, string &message);
bool list_protocol(int *socket, char *buffer, string &message);
bool read_protocol(int *socket, char *buffer, string &message);
bool delete_protocol(int *socket, char *buffer, string &message);

int main(int argc, char* argv[])
{
    int port;
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;

    if(argc < 3)
    {
        cerr << "error: no port or mail-spool-directoryname passed" << endl;
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);       //converts char* to int

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


// handles signals and closes connection controlled
void signalHandler(int sig)
{
    if( sig == SIGINT )
    {
        cout << "abort Requested...";
        abortRequested = true;

        // With shutdown() one can initiate normal TCP close sequence ignoring
        // the reference count.
        if(new_socket != -1)
        {
            if(shutdown(new_socket, SHUT_RDWR) == -1)
            {
                cerr << "shutdown new_socket" << endl;
            }
            if(close(new_socket) == -1)
            {
                cerr << "close new_socket" << endl;
            }
            new_socket = -1;
        }

        if(create_socket != -1)
        {
            if(shutdown(create_socket, SHUT_RDWR) == -1)
            {
                cerr << "shutdown create_socket" << endl;
            }
            if(close(create_socket) == -1)
            {
                cerr << "close create_socket" << endl;
            }
            create_socket = -1;
        }
    }
    else
    {
        exit(sig);
    }
}

void clientCommunication(void* data)
{
    char buffer[BUF];
    string message;
    int *current_socket = (int*) data;

    ////////////////////////////////////////////////////////////////////////////
    // SEND welcome message
    message = "Welcome to myserver!\r\nPlease enter your commands...\r\n";
    if(! send_client(current_socket, message))
    {
        return;
    }

    do
    {
        if( !handle(current_socket, buffer, message))
        {
            break;
        }


    } while(message != "QUIT" && !abortRequested);

    // closes/frees the descriptor if not already
    if (*current_socket != -1)
    {
        if (shutdown(*current_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown new_socket");
        }
        if (close(*current_socket) == -1)
        {
            perror("close new_socket");
        }
        *current_socket = -1;
    }

}

void print_usage(char* program_name)
{
    cout << "Usage: " << program_name << " <port> <mail-spool-directoryname>" << endl;
}

bool send_client(const int* socket, string &buffer)
{
    if(send(*socket, buffer.c_str(), buffer.length(), 0) == -1)
    {
        cerr << "send answer failed" << endl;
        return false;
    }
    return true;
}

bool receive_client(const int *socket, char *buffer, string &message)
{
    int size;
    /////////////////////////////////////////////////////////////////////////
    // RECEIVE
    size = recv(*socket, buffer, BUF - 1 , 0);
    if( size == -1 /* just to be sure */)
    {
        if(abortRequested)
        {
            cerr << "recv error after aborted " << endl;
        }
        else
        {
            cerr << "recv error" << endl;
        }
        return false;
    }

    if( size == 0 )
    {
        cout << "Client closed remote socket" << endl;
        return false;
    }

    // remove ugly debug message, because of the sent newline of client
    if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
    {
        size -= 2;
    }
    else if (buffer[size - 1] == '\n')
    {
        --size;
    }
    buffer[size] = '\0';

    message = buffer;   // handles buffer in string message

    return true;
}

bool handle(int *socket, char *buffer, string &message)
{
    if( !receive_client(socket, buffer, message))
    {
        return false;
    }

    cout << "Handle message: " << message << endl;

    if(message == "SEND")
    {
        if(!send_protocol(socket, buffer, message))
        {
            ERR(socket, message);
        }
        else
        {
            OK(socket, message);
        }
        return true;
    }
    else if(message == "LIST")
    {
        return list_protocol(socket, buffer, message);
    }
    else if(message == "READ")
    {
        return read_protocol(socket, buffer, message);
    }
    else if(message == "DELETE")
    {
        return delete_protocol(socket, buffer, message);
    }
    else if(message == "QUIT")
    {
        return false;
    }
    else
    {
        message = "invalid command";
        return send_client(socket, message);
    }
}

void OK(int *socket, string &message)
{
    message = "OK";
    send_client(socket, message);
}

void ERR(int *socket, string &message)
{
    message = "ERR";
    send_client(socket, message);
}

//// PROTOCOLS
bool send_protocol(int *socket, char *buffer, string &message)
{
    // get Sender
    string sender;
    if(receive_client(socket,buffer,message))
    {
        sender = message;
        if(sender.empty() || sender.length() > 8)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    // get Receiver
    string receiver;
    if(receive_client(socket,buffer,message))
    {
        receiver = message;
        if(receiver.empty() || receiver.length() > 8)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    // get Subject
    string subject;
    if(receive_client(socket,buffer,message))
    {
        subject = message;
        if(subject.empty() || subject.length() > 80)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    // get Message
    message = " ";      //to make sure message is != "."
    string mail_message;
    while(message != ".")
    {
        if(receive_client(socket,buffer,message))
        {
            mail_message += message;
        }
        else
        {
            return false;
        }
    }

    message = sender + "\n" + receiver + "\n" + subject + "\n" + mail_message + "\n";
    send_client(socket, message);

    return true;
}

bool list_protocol(int *socket, char *buffer, string &message)
{
    return true;
}

bool read_protocol(int *socket, char *buffer, string &message)
{
    return true;
}

bool delete_protocol(int *socket, char *buffer, string &message)
{
    return true;
}
