/*****************************************************************************/
/*** SMTP_server_thread.c                                                  ***/
/***                                                                       ***/
/*** Compile : gcc SMTP_server_thread.c -o SMTP_server_thread -lpthread    ***/
/*** Compile : gcc asi5.c -o asi5 -lpthread -I../src -L.. -liniparser      ***/
/*****************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>

/* Definations */
#define DEFAULT_BUFLEN 1024

/*Server should take this port number from ini file*/
//#define PORT 6862 sever can read it from ini file

/*Ini headers*/
#include <stdio.h>
#include <dictionary.h>
#include "iniparser.h"

/* Variables to hold query results */
const char  *   listen_IP ;
int             listen_port ;
const char  *   server_root ;
const char  *   server_name ;
const char  *   domain_name ;
const char  *   joe_user ;
const char  *   jane_user ;
const char  *   bob_user ;



void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }
void create_example_ini_file(void);
int  parse_ini_file(char * ini_name);

/*--------------------------------------------------------------------*/
/*--- Child -                                                      ---*/
/*--------------------------------------------------------------------*/
void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    
    char data[]="220 My Beautiful server <foo.com>\r\n";
    send(client, data,strlen(data),0);

    do
    {
    	
        bytes_read = recv(client, line, sizeof(line), 0);
        if (bytes_read > 0) {
                if ( (bytes_read=send(client, line, bytes_read, 0)) < 0 ) {
                        printf("Send failed\n");
                        break;
                }
        } else if (bytes_read == 0 ) {
                printf("Connection closed by client\n");
                break;
        } else {
                printf("Connection has problem\n");
                break;
        }
        
    } while (bytes_read > 0);
    close(client);
    return arg;
}

/*--------------------------------------------------------------------*/
/*--- main - setup server and await connections (no need to clean  ---*/
/*--- up after terminated children.                                ---*/
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{   int sd,opt,optval;
    struct sockaddr_in addr;
    unsigned short port=0;
    int     status ;
    
    status = parse_ini_file("Server.ini"); /*Function call of iniparser, which is reading from ini file*/

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
                port=atoi(optarg);
                break;
        }
    }


    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port > 0 )
                addr.sin_port = htons(port);
    else
                addr.sin_port = htons(listen_port);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("System ready on port %d\n",ntohs(addr.sin_port));

    while (1)
    {
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if ( pthread_create(&child, NULL, Child, &client) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }
    return 0;
}


/*I have to write code for file here*/
void create_example_ini_file(void)
{
    FILE    *   ini ;

    if ((ini=fopen("example.ini", "w"))==NULL) {
        fprintf(stderr, "iniparser: cannot create example.ini\n");
        return ;
    }
    fprintf(ini,"It will read from a file here");
    fclose(ini);
}

/*For ini parser*/
int parse_ini_file(char * ini_name)
{
    dictionary  *   ini ;

    ini = iniparser_load(ini_name);
    if (ini==NULL)
	{
	    fprintf(stderr, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }
    iniparser_dump(ini, stderr);

    /* Get Server attributes */
	listen_IP = iniparser_getstring(ini, "server:ListenIp", NULL);
    listen_port = iniparser_getint(ini, "server:ListenPort", -1);
    server_root = iniparser_getstring(ini, "server:ServerRoot", NULL);
    server_name = iniparser_getstring(ini, "server:ServerName", NULL);
    domain_name = iniparser_getstring(ini, "server:DomainName", NULL);
    
    /* Get User attributes */
    joe_user = iniparser_getstring(ini, "users:joe", NULL);
    jane_user = iniparser_getstring(ini, "users:jane", NULL);
    bob_user = iniparser_getstring(ini, "bob:joe", NULL);
    
    iniparser_freedict(ini);
    return 0 ;
}

