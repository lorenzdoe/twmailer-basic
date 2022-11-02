#ifndef BASIC_SERVER_UTILS_H
#define BASIC_SERVER_UTILS_H

#define BUF 1024

#include <string>

namespace twMailer
{
    using std::string;
}

using namespace twMailer;

extern bool abortRequested;
extern int create_socket;         //
extern int new_socket;            // hold the file descriptor
extern string spool;              // Mail directory

////// need global variables
void clientCommunication(void* data);
void signalHandler(int sig);
bool receive_client(const int *socket, char *buffer, string &message);

////////////////////////////////////////////////////////////////
bool send_client(const int* socket, string &buffer);   //sends client a string
bool handle(int *socket, char *buffer, string &message);

void OK(int *socket, string &message);
void ERR(int *socket, string &message);

////// PROTOCOLS
bool send_protocol(string &message);
bool list_protocol(string &message);
bool read_protocol(string &message);
bool delete_protocol(string &message);

////// OTHER UTILS
void print_usage(char* program_name);


#endif //BASIC_SERVER_UTILS_H
