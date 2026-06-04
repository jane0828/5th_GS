#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>

#include "miman_config.h"
#include "miman_imgui.h"
#include "miman_s_control.h"

extern Console console;
Sband * sgs;

int init_sband(Setup * setups)
{
    sgs = new Sband(setups->S_Address, setups->S_Username, setups->S_passwd);
    if(sgs->use == true)
        return 0;
    else
        return -1;
}

int verify_knownhost(ssh_session session){
    char *hexa;
    int state, rc;
    char buf[10];
    unsigned char *hash = NULL;
    size_t hlen;
    ssh_key srv_pubkey;
 
    state=ssh_session_is_known_server(session);
 
    console.AddLog("[DEBUG]##ssh_is_server_known : %d", state);
 
    switch(state){
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
        fprintf(stderr,"Host key for server changed : server's one is now :\n");
        ssh_print_hexa("Public key hash",hash, hlen);
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr,"For security reason, connection will be stopped\n");
        return -1;
    case SSH_SERVER_FOUND_OTHER:
        fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
        fprintf(stderr,"An attacker might change the default server key to confuse your client"
            "into thinking the key does not exist\n"
            "We advise you to rerun the client with -d or -r for more safety.\n");
        return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
        fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
        fprintf(stderr,"the file will be automatically created.\n");
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
    
        if (ssh_write_knownhost(session) < 0) {
            ssh_clean_pubkey_hash(&hash);
            fprintf(stderr, "error %s\n", strerror(errno));
            return -1;
        }
 
        break;
    case SSH_SERVER_ERROR:
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr,"%s",ssh_get_error(session));
        return -1;
    }
    ssh_clean_pubkey_hash(&hash);
    return 0;
}

int Sband::Initialize()
{
    this->s_session = ssh_new();
    memset(this->buffer, 0, sizeof(this->buffer));
    if(s_session == NULL)
    {
        console.AddLog("[ERROR]##S-Band Error : Cannot Load ssh sockets.");
        return -1;
    }

    int verbosity = SSH_LOG_PROTOCOL;
    int type = 1;
    unsigned int port = 22;

    ssh_options_set(s_session, SSH_OPTIONS_HOST, this->_IP);
    ssh_options_set(s_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(s_session, SSH_OPTIONS_PORT, &port);

    if( ssh_connect(s_session) != SSH_OK )
    {
        console.AddLog("[ERROR]##S-Band Error : Cannot Connect to S-Band GS.");
        return -1;
    }

    int state = verify_knownhost(s_session);
    if(state != 0)
        return -1;

    int rc;

    if (SSH_AUTH_METHOD_PASSWORD) 
    {

        rc = ssh_userauth_password(s_session, this->_Username, this->_Password);

        if (rc == SSH_AUTH_ERROR) 
        {
            console.AddLog("[ERROR]##S-Band Error : Authentication Error.");
            return rc;
        } 
        else if (rc == SSH_AUTH_SUCCESS) 
        {
            // console.AddLog("[OK]##Connect To S-Band GS.....Success.");;
        }
    }

    this->use = true;

    return SSH_OK;

}

int Sband::finSband()
{
    ssh_disconnect(this->s_session);
    ssh_free(this->s_session);
    return SSH_OK;
}

int Sband::cmd(char * _command)
{
    this->s_channel = ssh_channel_new(this->s_session);
    int rc, nbytes;
    sprintf(command, _command, sizeof(command));
    memset(this->buffer, 0, sizeof(this->buffer));

    if ( this->s_channel == NULL )
    {
        ssh_disconnect(s_session);

        console.AddLog("[ERROR]##S-Band Error : Cannot Create SSH Channel.");

        return SSH_ERROR;
    }

    // SSH 채널 열기 
    if ( ssh_channel_open_session(this->s_channel) != SSH_OK )
    {
        ssh_disconnect(s_session);
        console.AddLog("[ERROR]##S-Band Error : Cannot Open SSH Channel.");
        return SSH_ERROR;
    }

    // 원격 터미널 요청
    if( ssh_channel_request_pty_size(this->s_channel, "vt220", 80, 24) != SSH_OK )
    {
        ssh_disconnect(s_session);

        console.AddLog("[ERROR]##S-Band Error : Cannot Create SSH Channel.");

        return SSH_ERROR;
    }

    
    rc = ssh_channel_request_exec(this->s_channel, command);
    if (rc != SSH_OK)
    {
        ssh_channel_close(this->s_channel);
        ssh_channel_free(this->s_channel);
        return rc;
    }
    
    nbytes = ssh_channel_read(this->s_channel, buffer, sizeof(buffer), 0);
    clock_t start = clock();
    clock_t now;
    while (nbytes > 0)
    {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes)
        {
        ssh_channel_close(this->s_channel);
        ssh_channel_free(this->s_channel);
        return SSH_ERROR;
        }
        nbytes = ssh_channel_read(this->s_channel, buffer, sizeof(buffer), 0);
        
        now = clock();
        if((double)(now - start) / CLOCKS_PER_SEC > 3.0f)
            break;
        
    }
    if (nbytes < 0)
        {
            ssh_channel_close(this->s_channel);
            ssh_channel_free(this->s_channel);
            return SSH_ERROR;
        }
    
    ssh_channel_send_eof(this->s_channel);
    ssh_channel_close(this->s_channel);
    ssh_channel_free(this->s_channel);

    int i = 0;
    while(this->buffer[i])
        i++;
    this->buffer[i] = '\0';
    
    return SSH_OK;
}