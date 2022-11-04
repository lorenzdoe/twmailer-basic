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

// creates the corresponding folders and meta file
// saves a message in sender, receiver folder
bool Mail::save(string& spool)
{
    string mail = "sender:" + sender + "\n"
              + "receiver:" + receiver + "\n"
              + "subject:" + subject + "\n"
              + "message:\n"
              + message + "\n";

    // meta-data file holds id of messages
    string metaSender = "._" + sender + "__.txt";
    string metaReceiver = "._" + receiver + "__.txt";
    try
    {
        // creates directory if not exists
        fs::create_directory(sender = spool + "/" + sender);
        fs::create_directory(receiver = spool + "/" + receiver);
    }
    catch(...)
    {
        return false;
    }

    string s_number;    // message id sender
    string r_number;    // message id receiver

    // if no meta file exists create one
    if(!fs::exists(spool + "/" + metaSender))
    {
        std::ofstream MetaSenderFile( spool + "/" + metaSender);
        MetaSenderFile << "0";
        MetaSenderFile.close();

        s_number = "0";
    }
    // if meta file exists, read out id of last saved mail and increment
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