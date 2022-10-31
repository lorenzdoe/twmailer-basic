#include <filesystem>
#include <fstream>
#include "../header/Mail.h"

namespace fs = std::filesystem;

Mail::Mail() = default;
Mail::~Mail() = default;

Mail::Mail(string& sender, string& receiver, string& subject, string& message)
{
    this->sender = sender;
    this->receiver = receiver;
    this->subject = subject;
    this->message = message;
}

void Mail::save(string& spool)
{
    string mail = sender + "\n"
              + receiver + "\n"
              + subject + "\n"
              + message + "\n";

    // Search, create and manipulate files
    // first creates directory if not exists
    fs::create_directory(sender = spool+"/"+sender);
    fs::create_directory(receiver = spool+"/"+receiver);

    std::ofstream SendersFile(sender + "/" + subject + ".txt");
    std::ofstream ReceiversFile(receiver + "/" + subject + ".txt");
    SendersFile << mail;
    ReceiversFile << mail;
    SendersFile.close();
    ReceiversFile.close();
}