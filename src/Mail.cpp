#include <filesystem>
#include <fstream>
#include "../header/Mail.h"

namespace fs = std::filesystem;

Mail::Mail() = default;
Mail::~Mail() = default;

Mail::Mail(string &sender, string &receiver, string &subject, string &message)
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
    // directory contains a metadata file
    string metaSender = "__" + sender + "__.txt";
    string metaReceiver = "__" + receiver + "__.txt";
    try
    {
        fs::create_directory(sender = spool + "/" + sender);
        fs::create_directory(receiver = spool + "/" + receiver);
    }
    catch(...)
    {
        return false;
    }

    string s_number;    // message number sender
    string r_number;    // message number receiver

    if(!fs::exists(spool + "/" + metaSender))
    {
        std::ofstream MetaSenderFile( spool + "/" + metaSender);
        MetaSenderFile << "0";
        MetaSenderFile.close();

        s_number = "0";
    }
    else
    {
        std::ifstream ReadNumber(spool + "/" + metaSender);
        getline(ReadNumber,s_number);
        ReadNumber.close();
        int number = stoi(s_number);
        ++number;
        s_number = std::to_string(number);
        std::ofstream MetaSenderFile( spool + "/" + metaSender, std::ofstream::trunc);
        MetaSenderFile << s_number;
        MetaSenderFile.close();
    }

    if(!fs::exists(spool + "/" + metaReceiver))
    {
        std::ofstream MetaReceiverFile( spool + "/" + metaReceiver);
        MetaReceiverFile << "0";
        MetaReceiverFile.close();

        r_number = "0";
    }
    else
    {
        std::ifstream ReadNumber(spool + "/" + metaReceiver);
        getline(ReadNumber,r_number);
        ReadNumber.close();
        int number = stoi(r_number);
        ++number;
        r_number = std::to_string(number);
        std::ofstream MetaReceiverFile( spool + "/" + metaReceiver, std::ofstream::trunc);
        MetaReceiverFile << r_number;
        MetaReceiverFile.close();
    }

    std::ofstream SendersFile(sender + "/" + s_number + "_" + subject + ".txt");
    SendersFile << mail;
    SendersFile.close();

    std::ofstream ReceiversFile(receiver + "/" + r_number + "_" + subject + ".txt");
    ReceiversFile << mail;
    ReceiversFile.close();

    return true;
}