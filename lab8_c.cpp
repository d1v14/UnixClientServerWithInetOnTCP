#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/time.h>
#include <csignal>
#include <vector>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct itimerval value;		
struct sockaddr_in serverAddr;

int conServerFlag,sendRequestFlag,getAnswerFlag;
int checkCon,clientSock;
pthread_t con,req,get;


using namespace std;

void sigHandler(int signo)
{
	shutdown(clientSock,SHUT_RDWR);
	close(clientSock);
	conServerFlag=1;
	sendRequestFlag=1;
	getAnswerFlag=1;	
	printf("Сервер прекратил работу,нажмите любую клавишу для завершения.\n");		
}

void* sendRequest(void *val)
{
	int i=1;
	while(sendRequestFlag!=1)
	{
		int checkSend=send(clientSock,&i,sizeof(i),0);
		if(checkSend==-1)
		{
			perror("Ошибка отправки");
		}
		else
		{
			printf("Запрос №%i отправлен на сервер\n",i);
			i++;	
		}
		sleep(1);
	}
	pthread_exit(NULL);
		
}
void* getAnswer(void *val)
{
	int i=1;
	int buf;
	while(getAnswerFlag!=1)
	{
		int checkRec=recv(clientSock,&buf,sizeof(buf),0);
		if(checkRec==-1)
		{
			perror("Ошибка приема");
			sleep(1);
		}
		else
		{
			if(checkRec==0)
			{
				shutdown(clientSock,SHUT_RDWR);
			}
			else
			{
				printf("По %i запросу получен ответ:%i\n",i,buf);
				i++;
			}
	
		}
	}
	pthread_exit(NULL);

}
void* conServer(void *val)
{
	struct sockaddr_in addr;
	while(conServerFlag !=1)
	{
		checkCon=connect(clientSock,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
		if(checkCon==-1)
		{
			perror("Exchange socket");
			printf("\n");
			sleep(1);
		}
		else
		{
			printf("Подключение к серверу успешно\n");
			pthread_create(&req,NULL,sendRequest,NULL);
			pthread_create(&get,NULL,getAnswer,NULL);
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
}



	int main()
	{
		printf("Клиент начал работу\n");
		signal(SIGPIPE,sigHandler);
		
		serverAddr.sin_family=AF_INET;
		serverAddr.sin_port=htons(7000);
		serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
				
		int optval=1;
		clientSock=socket(AF_INET,SOCK_STREAM,0);
		setsockopt(clientSock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
		pthread_create(&con,NULL,conServer,NULL);
		printf("Программа ждет нажатия клавиши\n");
		getchar();
		printf("Клавиша нажата\n");
		conServerFlag=1;
		sendRequestFlag=1;
		getAnswerFlag=1;
		pthread_join(con,NULL);
		pthread_join(req,NULL);
		pthread_join(get,NULL);
		shutdown(clientSock,SHUT_RDWR);
		close(clientSock);
		printf("Программа завершила работу.");
		
	}
