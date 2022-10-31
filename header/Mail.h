//
// Created by lorenz on 31.10.22.
//

#ifndef BASIC_MAIL_H
#define BASIC_MAIL_H

#include <string>

using std::string;

class Mail
{
private:
    string sender;
    string receiver;
    string subject;
    string message;

public:
    Mail();
    Mail(string& sender, string& receiver, string& subject, string& message);
    ~Mail();

    bool save(string& spool);
};


#endif //BASIC_MAIL_H
