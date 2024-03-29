/*****************************************************************************/
/*** SMTP_server_thread.c                                                  ***/
/***                                                                       ***/
/*** Compile : gcc SMTP.c -o smtp -lpthread -I../src -L.. -liniparser      ***/
/***                                                                       ***/
/*** git clone https://github.com/Abdulwarissherzad/asi5.git               ***/
/*** Copyright (c) 2020 Abdul waris sherzad.                               ***/
/*****************************************************************************/

#include <stdio.h>
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
#include <dictionary.h>
#include "iniparser.h"

/*For time*/
#include <time.h>

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

int  parse_ini_file(char * ini_name);
int indexOf(FILE *fptr, const char *word, int *lin, int *col);

/*Char array variables to hold query results*/
const char  *   listen_IP ;
int             listen_port ;
char ser_root[40]=" ";
char ser_name[40]=" ";
char dom_name[10]=" ";
char joe_user1[50]=" ";
char jane_user2[50]=" ";
char bob_user3[50]=" ";

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
    
	char * rec_email;
	char * rec_domain;
    char * rec_username;
    char * rec_user_domain;

    char data[150];
    char data1[150];
	char re_user[5] ="";
    char format[150] ="";
    char re_email[10]="";
    char format1[160] ="";
    char domain_cpy[10]="";
    
	 // Greating message send to client
	sprintf(data,"220 %s <%s>\n",ser_name,dom_name);
    send(client, data,strlen(data),0);
    memset(data,0,strlen(data));
	
    do{
    	
		memset(line,0,strlen(line));
        bytes_read = recv(client, line, sizeof(line), 0);
      
	    if (strcmp(line,"NOOP\n")==0){
            send(client,"250 Ok\n",8,0);
		}
		else if(strcmp(line,"RSET\n") ==0  )
		{
	        send(client,"250 Reset State Ok\n",20,0);
			memset(line,0,strlen(line));
	        memset(re_user,0,strlen(re_user));
	        memset(data1,0,strlen(data1));
		}
		
    	else if(strstr(line,"HELO ") !=0 )
		{
            rec_domain = strtok(line+5,"\n");
    
            /*Reading from a file for ban domain*/
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
    	
        else if(strstr(line,"MAIL FROM: ") != 0)
		{
        	rec_email = strtok(line+11,"\n");

        	strcpy(re_email,rec_email);

			/*Reading and checking from a file for ban email*/
            fptr = fopen("ban_email.cfg", "r");

            // Find index of word in fptr
             indexOf(fptr, rec_email, &lin, &col);

            if (lin != -1 ){

	            sprintf(data1,"Sorry! your email is banned, you can not contenue...\n");
	            send(client, data1,strlen(data1),0);
	            close(client);
     	    }
			 else
			{
            	sprintf(data1,"250 %s ... Sender ok\n",rec_email);
            	send(client, data1,strlen(data1),0);
	        }
     	       fclose(fptr);
		}
		
		else if(strstr(line,"RCPT TO: ") !=0 )
		{

			rec_username = strtok(line+9,"@");
			
			memset(re_user,0,strlen(re_user));
			strcpy(re_user,rec_username);
			rec_user_domain = strtok(NULL," ");
			strcpy(domain_cpy,rec_user_domain);


		    if((strcmp(rec_username,"joe") !=0) && (strcmp(rec_username,"jane") !=0) && (strcmp(rec_username,"bob") !=0))
			{
				//Note: If it was correct it will execute else otherwise inside if we execute
				sprintf(data1,"Recipient user name is not exist!\n");
             	send(client, data1,strlen(data1),0);
			}
		    else if ( strstr(domain_cpy,dom_name) == 0 ){
				sprintf(data1,"Recipient domain is not exist!\n");
             	send(client, data1,strlen(data1),0);
			}
			else
			{
				sprintf(data1,"250 %s@foo.com ... Recipient ok\n",rec_username);
            	send(client, data1,strlen(data1),0);
			}
		}
		
		else if(strstr(line,"DATA") != 0)
		{
			/*First Condition is for validation*/
			if((strcmp("",re_user)== 0) && (strcmp("",re_email)==0))//validation
            {
				send(client,"You Missed One Step!\n",22,0);
            }
			else
			{
				sprintf(data1,"354 Enter mail, end with \".\" on a line by itself\n");
	            send(client, data1,strlen(data1),0);
	            /*For time */
	            time_t ti = time(NULL);
	            struct tm tm = *localtime(&ti);

				char file_write[100];
			    if(strcmp("joe",re_user)==0){

	                sprintf(file_write,"%s%s/%02d%02d%02d%02d%02d.mbox",ser_root,joe_user1,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
				}else if(strcmp("jane",re_user)==0){

					sprintf(file_write,"%s%s/%02d%02d%02d%02d%02d.mbox",ser_root,jane_user2,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
				}else if(strcmp("bob",re_user)==0){

					sprintf(file_write,"%s%s/%02d%02d%02d%02d%02d.mbox",ser_root,bob_user3,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
				}
				
				fptr = fopen(file_write, "w");
				sprintf(format,"Return-Path: < %s >\nReceived: from host.ciu.edu.tr ([212.175.150.10])\n\tby db.ciu.edu.tr with SMTP\n\tfor < %s@foo.com >\n",re_email,re_user);
				strcpy(format1,format);
				fputs(format1,fptr);
			    do{
			        memset(line,0,strlen(line));
			        bytes_read = recv(client, line, sizeof(line), 0);
	                fputs(line,fptr);
				}while(strncmp(line,".\n",sizeof(".\n")) != 0);
				fclose(fptr);
				sprintf(line,"250 Message accepted for delivery\n");
	            send(client, line,strlen(line),0);
            }
		}
		else if((strcmp(line,"QUIT\n")==0 )){
            send(client,"221 foo.com closing connection\n",33,0);
			close (client);
		}
		else
	    send(client, "Try again you enter wrong commend\n",35,0);
	    
    } while (bytes_read > 0 );
    close (client);
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
    
    /*Function call of iniparser, which is reading from ini file*/
    int     status ;
    status = parse_ini_file("Server.ini"); 

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

	if(strncmp(listen_IP,"All",strlen("All")))
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

/*-----------------------------------------------------------------------*/
/*--- Function for checking for band domain and band email from a fil ---*/
/*-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------*/
/*---                Function for For ini parser                      ---*/
/*-----------------------------------------------------------------------*/
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
    
	/*we copied const char to char because outside the function we can take value of cons char*/
    /* Get User attributes */
    joe_user = iniparser_getstring(ini, "users:joe", NULL);
    sprintf(joe_user1 ,"%s",joe_user);
    jane_user = iniparser_getstring(ini, "users:jane", NULL);
    sprintf(jane_user2 ,"%s",jane_user);
    bob_user = iniparser_getstring(ini, "users:bob", NULL);
    sprintf(bob_user3 ,"%s",bob_user);

    
    iniparser_freedict(ini);
    return 0 ;
}


