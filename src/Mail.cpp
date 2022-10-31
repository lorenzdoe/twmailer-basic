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

bool Mail::save(string& spool)
{
    string mail = sender + "\n"
              + receiver + "\n"
              + subject + "\n"
              + message + "\n";

    // Search, create and manipulate files
    // first creates directory if not exists
    try
    {
        fs::create_directory(sender = spool+"/"+sender);
        fs::create_directory(receiver = spool+"/"+receiver);
    }
    catch(...)
    {
        return false;
    }

    // check if message with this subject already exists
    while(fs::exists(sender + "/" + subject + ".txt") || fs::exists(receiver + "/" + subject + ".txt"))
    {
        subject += "_1";
        if(subject.length() > 80)
        {
            return false;
        }
    }

    std::ofstream SendersFile(sender + "/" + subject + ".txt");
    std::ofstream ReceiversFile(receiver + "/" + subject + ".txt");
    SendersFile << mail;
    ReceiversFile << mail;
    SendersFile.close();
    ReceiversFile.close();

    return true;
}