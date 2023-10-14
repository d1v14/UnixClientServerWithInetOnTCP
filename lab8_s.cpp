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
using namespace std;

struct itimerval value;		
struct sockaddr_in serverAddr;
vector <long int> v1;
int waitConFlag,getRequestFlag,requestHandlerFlag;
int exchange,listener;
pthread_mutex_t mut1;
pthread_t req,lis,han;	


void* getRequest(void *val)
{
	int buf;
	while(getRequestFlag!=1)
		{ 
			int checkRec=recv(exchange,&buf,sizeof(buf),0);
			if(checkRec==-1)
			{
				perror("Ошибка приема");
				sleep(1);
			}
			else
			{
				if(checkRec==0)
				{
					printf("Клиент отключился.\n");
					shutdown(exchange,SHUT_RDWR);
					sleep(1);
				}
				else
				{
					printf("Принят запрос №%i от клиента\n",buf);
					pthread_mutex_lock(&mut1);
					v1.push_back(buf);
					pthread_mutex_unlock(&mut1);
					sleep(1);
				}
		}
	}
	pthread_exit(NULL);

}

void* requestHandler(void *val)
{
	int buf;
	while(requestHandlerFlag!=1)
	{
		pthread_mutex_lock(&mut1);
		if(v1.empty()!=1)
		{
			int reqNumber=v1.front();
			v1.erase(v1.begin());
			pthread_mutex_unlock(&mut1);
			getitimer(ITIMER_REAL,&value);
			buf=value.it_value.tv_sec;
			int checkSend=send(exchange,&buf,sizeof(buf),0);
			if(checkSend==-1)
			{
				perror("Ошибка отправки клиенту");	
				sleep(1);			
			}
			else
			{
				printf("По запросу №%i клиента отправлено значение %i\n",reqNumber,buf);
			}
			
		}
		else
		{
			pthread_mutex_unlock(&mut1);
			sleep(1);
		}
	}
	pthread_exit(NULL);
	
}

void* waitCon(void *val)
{
	struct sockaddr addr;
	socklen_t addrlen=sizeof(addr);
	while(waitConFlag !=1)
	{	
		exchange=accept(listener,&addr,&addrlen);
		if(exchange==-1)
		{
			perror("Ошибка подключения");
			sleep(1);
		}
		else
		{
			printf("Клиент подкючился к серверу\n");
			pthread_create(&req,NULL,getRequest,NULL);
			pthread_create(&han,NULL,requestHandler,NULL);
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
}



int main()
	{
		printf("Сервер начал работу\n");
		int optval=1;
		pthread_mutex_init(&mut1,NULL);
		listener=socket(AF_INET,SOCK_STREAM,0);
		fcntl(listener,F_SETFL,O_NONBLOCK);	
		serverAddr.sin_family=	AF_INET;
		serverAddr.sin_port=	htons(7000);
		serverAddr.sin_addr.s_addr=	inet_addr("127.0.0.1");
		
		value.it_value.tv_sec=240;
		value.it_value.tv_usec=0;
		setitimer(ITIMER_REAL,&value,NULL);
		int con = bind(listener,(sockaddr*)&serverAddr,sizeof(serverAddr));		
		
		if(con==-1)
		{
			perror("Ошибка привязки");
		}
		
		listen(listener,sizeof(int));	
				
		setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
		
		pthread_create(&lis,NULL,waitCon,NULL);
		printf("Программа ждет нажатия клавиши\n");
		getchar();
		printf("Клавиша нажата\n");
		waitConFlag=1;
		getRequestFlag=1;
		requestHandlerFlag=1;
		pthread_join(lis,NULL);
		pthread_join(req,NULL);
		pthread_join(han,NULL);
		pthread_mutex_destroy(&mut1);
		shutdown(listener,SHUT_RDWR);
		shutdown(exchange,SHUT_RDWR);
		close(listener);
		close(exchange);
		printf("Программа завершила работу.");		
	}
