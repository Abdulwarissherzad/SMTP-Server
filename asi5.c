/*****************************************************************************/
/*** SMTP_server_thread.c                                                  ***/
/***                                                                       ***/
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
#define BUFFER_SIZE 1000


/*Ini headers*/
#include <stdio.h>
#include <dictionary.h>
#include "iniparser.h"

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

int  parse_ini_file(char * ini_name);
int indexOf(FILE *fptr, const char *word, int *lin, int *col);

/*We can not chang the content of cons char *, so we     */
/*coppy it to char array variables to hold query results*/
const char  *   listen_IP ;
int             listen_port ;
char ser_root[40]=" ";
char ser_name[40]=" ";
char  dom_name[10]=" ";
char joe_user1[50]= " ";
char jane_user2[50]= " ";
char bob_user3[50]= " ";

/*For reading of a file from text*/
FILE *fptr;
char  word[50] ;
int lin, col;


/*--------------------------------------------------------------------*/
/*--- Child -                                                      ---*/
/*--------------------------------------------------------------------*/

void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    char * rec_domain;
    
    char data[DEFAULT_BUFLEN];
    char data1[DEFAULT_BUFLEN];


    do
    {
    sprintf(data,"220 %s <%s>\n",ser_name,dom_name);
    send(client, data,strlen(data),0);
    memset(data,0,strlen(data));
    
    recv(client, line, sizeof(line), 0);
	if(strstr(line,"HELO ") !=0){
    rec_domain = strtok(line+5,"\n");
    
    
    /*Reading from a file for band*/
    fptr = fopen("ban_domain.cfg", "r");

    // Find index of word in fptr
    indexOf(fptr, rec_domain, &lin, &col);
    
    if (lin != -1 ){
	    sprintf(data1,"Sorry! your domain is banned, you can not contenue...\n");
	    send(client, data1,strlen(data1),0);
	    close(client);
	}else{
	sprintf(data1,"250 Hello %s, pleased to meet you\n",rec_domain);
	send(client, data1,strlen(data1),0);
	}
	fclose(fptr);
	}
	else
	{
		sprintf(data1,"Try again you enter wrong commend:%s",line);
		send(client, data1,strlen(data1),0);
		
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

	if(strncmp(listen_IP,"ALL",strlen("All")))
    addr.sin_addr.s_addr = INADDR_ANY;
    else
    PANIC("Ip Address");

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


/*Read band domain and band email from a file*/
int indexOf(FILE *fptr, const char *word, int *lin, int *col)
{
    char str[BUFFER_SIZE];
    char *pos;

    *lin = -1;
    *col  = -1;

    while ((fgets(str, BUFFER_SIZE, fptr)) != NULL)
    {
        *lin += 1;

        // Find first occurrence of word in str
        pos = strstr(str, word);

        if (pos != NULL)
        {
            // First index of word in str is
            // Memory address of pos - memory
            // address of str.
            *col = (pos - str);
            break;
        }
    }


    // If word is not found then set line to -1
    if (*col == -1)
        *lin = -1;

    return *col;
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
    /* Variables to hold query results */
	const char  *   server_name ;
    const char  *   domain_name ;
    const char  *   server_root ;
    const char  *   joe_user ;
    const char  *   jane_user ;
    const char  *   bob_user ;
    
    /* Get Server attributes */
	listen_IP = iniparser_getstring(ini, "server:ListenIp", NULL);
    listen_port = iniparser_getint(ini, "server:ListenPort", -1);
    server_root = iniparser_getstring(ini, "server:ServerRoot", NULL);
    strcpy(ser_root ,server_root);
    server_name = iniparser_getstring(ini, "server:ServerName", NULL);
    strcpy(ser_name ,server_name);
    domain_name = iniparser_getstring(ini, "server:DomainName", NULL);
    strcpy(dom_name ,domain_name);

    /* Get User attributes */
    joe_user = iniparser_getstring(ini, "users:joe", NULL);
    sprintf(joe_user1 ,"%s",joe_user);
    jane_user = iniparser_getstring(ini, "users:jane", NULL);
    sprintf(jane_user2 ,"%s",jane_user);
    bob_user = iniparser_getstring(ini, "bob:joe", NULL);
    //sprintf(bob_user3 ,"%s",bob_user);
    
    iniparser_freedict(ini);
    return 0 ;
}

