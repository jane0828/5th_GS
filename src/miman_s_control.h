#pragma once
#ifndef __MIMAN_S_CONTROL_H_
#define __MIMAN_S_CONTROL_H_
#include <string.h>
#include <iostream>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>

//Custom Headers
#include "miman_config.h"

int init_sband(Setup * setups);
int verify_knownhost(ssh_session session);

class Sband{
private :
    


public :
    char buffer[8192];
    char command[1024];
    ssh_session s_session;
    ssh_channel s_channel;
    char _IP[30];
    char _Username[30];
    char _Password[30];
    bool use = false;

    Sband(char * IP, char * Username, char * Password)
    {
        this->~Sband();
        memcpy(_IP, IP, strlen(IP));
        memcpy(_Username, Username, strlen(Username));
        memcpy(_Password, Password, strlen(Password));
        int rc = this->Initialize();
        if(rc == 0)
            this->use = true;
    };

    ~Sband()
    {
        memset(_IP, 0, sizeof(_IP));
        memset(_Username, 0, sizeof(_Username));
        memset(_Password, 0, sizeof(_Password));
        memset(&s_session, 0, sizeof(s_session));
        this->use = false;
    };

    int Initialize();
    int finSband();
    int cmd(char * command);
};

#endif