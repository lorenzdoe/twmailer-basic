/*
 * tw-mailer basic
 *
 * CLIENT
 */
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

/* *************************************************** */

#define BUF 1024

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void print_usage(char* program_name);

bool read_buffer(char* buffer);
bool send_protocol(const int* socket, char* buffer);
bool list_protocol(const int* socket, char* buffer);
bool read_protocol(const int* socket, char* buffer);
bool delete_protocol(const int* socket, char* buffer);

int main(int argc, char *argv[])
{
    int port;
    char* ip_string;

    int create_socket;              // holds the file descriptor
    char buffer[BUF];
    struct sockaddr_in address;     // struct describing an internet socket address
                                    // documentation in 02sockets.pdf
    int size;
    bool isQuit;

    if(argc < 3)
    {
        cerr << "error: no IP or port passed" << endl;
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    ip_string = argv[1];
    port = atoi(argv[2]);       //converts char* to int

    ////////////////////////////////////////////////////////////////////////////
    // CREATE A SOCKET

    // AF_INET      - IPv4 Protocoll
    // SOCK_STREAM  - sequenced, reliable, two-way, connection-based byte streams
    if((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        cerr << "Socket error" << endl;
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // INIT ADDRESS

    // Attention: network byte order => big endian
    memset(&address, 0, sizeof(address));             // function from cstring -- init storage with 0
    address.sin_family = AF_INET;                            // IPv4
    address.sin_port = htons(port);                 // htons converts to network byte order


    inet_aton(ip_string, &address.sin_addr);                  //converts ip address from string notation to network stream

    ////////////////////////////////////////////////////////////////////////////
    // CREATE A CONNECTION

    if( connect(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1 )
    {
        cerr << "Connection error - no server available" << endl;
        return EXIT_FAILURE;
    }

    // inet_ntoa() converts internet addr from network byte order to string in IPv4 format
    cout << "Connection with the server (" << inet_ntoa(address.sin_addr ) << ") established" << endl;

    ////////////////////////////////////////////////////////////////////////////
    // RECEIVE DATA
    size = recv(create_socket, buffer, BUF-1 , 0);
    if(size == -1)
    {
        cerr << "recv error" << endl;
    }
    else if(size == 0)
    {
        cout << "Server closed remote socket" << endl;
    }
    else
    {
        buffer[size] = '\0';
        cout << buffer << endl;
    }

    do
    {
        cout << ">> ";

            if(!read_buffer(buffer))    //read buffer
            {
                break;
            }

            isQuit = strcmp(buffer, "QUIT") == 0;

            if(strcmp(buffer, "SEND") == 0)
            {
                if(!send_protocol(&create_socket, buffer))
                {
                    cout << "error: bad input" << endl;
                    continue;
                }
            }
            else if(strcmp(buffer, "LIST") == 0)
            {
                if(!list_protocol(&create_socket, buffer))
                {
                    cout << "error: bad input" << endl;
                    continue;
                }
            }
            else if(strcmp(buffer, "READ") == 0)
            {
                if(!read_protocol(&create_socket, buffer))
                {
                    cout << "error: bad input" << endl;
                    continue;
                }
            }
            else if(strcmp(buffer, "DEL") == 0)
            {
                if(!delete_protocol(&create_socket, buffer))
                {
                    cout << "error: bad input" << endl;
                    continue;
                }
            }
            else
            {
                //////////////////////////////////////////////////////////////////////
                // SEND DATA
                // send will fail if connection is closed, but does not set
                // the error of send, but still the count of bytes sent
                if(send(create_socket, buffer, strlen(buffer), 0) == -1)
                {
                    cerr << "ERROR: sending error" << endl;
                    break;
                }
            }



            //////////////////////////////////////////////////////////////////////
            // RECEIVE FEEDBACK
            size = recv(create_socket, buffer, BUF-1, 0);
            if(size == -1)
            {
                cerr << "recv error" << endl;
                break;
            }
            else if(size == 0)
            {
                cout << "Server closed remote socket" << endl;
                break;
            }
            else
            {
                buffer[size] = '\0';
                cout << "<< " << buffer << endl;
            }

    } while(!isQuit);

    ////////////////////////////////////////////////////////////////////////////
    // CLOSES THE DESCRIPTOR
    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            // invalid in case the server is gone already
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

void print_usage(char* program_name)
{
    cout << "Usage: " << program_name << " <ip> <port>" << endl;
}

bool read_buffer(char* buffer)
{
    int size;
    if(fgets(buffer, BUF, stdin) != nullptr)
    {
        size = strlen(buffer);

        // remove new-line signs from string at the end
        if( buffer[size-2] == '\r' && buffer[size-1] == '\n')
        {
            size -= 2;
            buffer[size] = 0;
        }
        else if(buffer[size-1] == '\n')
        {
            --size;
            buffer[size] = 0;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool send_protocol(const int* socket, char* buffer)
{
    // get Sender
    cout << "sender:\t\t";
    string sender;
    if(read_buffer(buffer))
    {
        sender = buffer;
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
    cout << "receiver:\t";
    string receiver;
    if(read_buffer(buffer))
    {
        receiver = buffer;
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
    cout << "subject:\t";
    string subject;
    if(read_buffer(buffer))
    {
        subject = buffer;
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
    cout << "message:"<< endl;
    string mail_message;
    while(strcmp(buffer, ".") != 0)
    {
        if(!read_buffer(buffer))
        {
            return false;

        }
        else
        {
            mail_message += (string)buffer + "\n";
        }
    }
    mail_message.pop_back();
    mail_message.pop_back();

    string mail = "SEND\n"
                  + sender + "\n"
                  + receiver + "\n"
                  + subject + "\n"
                  + mail_message;

    if(send(*socket, mail.c_str(), mail.length(), 0) == -1)
    {
        cerr << "send error" << endl;
        return false;
    }
    return true;
}

bool list_protocol(const int* socket, char* buffer)
{
    // get Username
    cout << "username: ";
    string command = "LIST\n";
    if(read_buffer(buffer))
    {
        if(buffer[0] == '\0')
        {
            return false;
        }
        command += buffer;
    }
    else
    {
        return false;
    }

    if(send(*socket, command.c_str(), command.length(), 0) == -1)
    {
        cerr << "list error" << endl;
        return false;
    }
    return true;
}

bool read_protocol(const int* socket, char* buffer)
{
    // get Username
    string command = "READ\n";
    cout << "username: ";
    if(read_buffer(buffer))
    {
        if(buffer[0] == '\0')
        {
            return false;
        }
        command += buffer;
        command += '\n';
    }
    else
    {
        return false;
    }

    cout << "Message-Nr: ";
    if(read_buffer(buffer))
    {
        if(buffer[0] == '\0')
        {
            return false;
        }
        command += buffer;
    }
    else
    {
        return false;
    }

    if(send(*socket, command.c_str(), command.length(), 0) == -1)
    {
        cerr << "read error" << endl;
        return false;
    }
    return true;
}

bool delete_protocol(const int* socket, char* buffer)
{
    // get Username
    string command = "DEL\n";
    cout << "username: ";
    if(read_buffer(buffer))
    {
        if(buffer[0] == '\0')
        {
            return false;
        }
        command += buffer;
        command += '\n';
    }
    else
    {
        return false;
    }

    cout << "Message-Nr: ";
    if(read_buffer(buffer))
    {
        if(buffer[0] == '\0')
        {
            return false;
        }
        command += buffer;
    }
    else
    {
        return false;
    }

    if(send(*socket, command.c_str(), command.length(), 0) == -1)
    {
        cerr << "delete error" << endl;
        return false;
    }
    return true;
}