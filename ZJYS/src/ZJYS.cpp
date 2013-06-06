/*
 ============================================================================
 Name        : ZJYS.cpp
 Author      : jiyuan zhao
 Version     :
 Copyright   : Your copyright notice
 Description : test in C++,
 ============================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#include <iostream>

#include "zjy.helloworld.pb.h"

using namespace std;
using namespace google;

#define MAXLINE 4096

class ExampleSearchService : public zjy::SearchService {
 public:
  void Search(protobuf::RpcController* controller,
              const zjy::request* request,
              zjy::response* response,
              protobuf::Closure* done) {
    if (request->args() == "google") {
      response->set_result("http://www.google.com");
    } else if (request->args() == "protocol buffers") {
      response->set_result("http://protobuf.googlecode.com");
    }
    done->Run();
  }
};

class ServerDone :public protobuf::Closure{
public:
	ServerDone(int connfd,zjy::response* res){
		this->connfd = connfd;
		this->resp = res;
	}
	void Run(){
		cout<<resp->status()<<endl;
		cout<<resp->result()<<endl;
		cout<<resp->opt()<<endl;
		//char array[100];
		resp->SerializeToFileDescriptor(connfd);
		//resp->SerializeToArray(array,100);
		//write(connfd,array,sizeof(array));
	}
private:
	int connfd;
	zjy::response* resp;
};

int main(int argc, char** argv) {
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	char buff[4096];
	int n;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY );
	servaddr.sin_port = htons(6666);

	if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	if (listen(listenfd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	printf("======waiting for client's request======\n");
	while (1) {
		if ((connfd = accept(listenfd, (struct sockaddr*) NULL, NULL)) == -1) {
			printf("accept socket error: %s(errno: %d)", strerror(errno),
					errno);
			continue;
		}
		/**
		n = recv(connfd, buff, MAXLINE, 0);
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		*/
		zjy::request* req = new zjy::request;

		//fstream in(connfd, ios::in | ios::binary);
        /**
        if (!req->ParseFromFileDescriptor(connfd)) {
             cerr << "Failed to parse helloworld." << endl;
             return -1;
        }
        */
		char array[100];
		int size = read(connfd,array,100);
		cout<<"read size:"<<size<<endl;
		req->ParseFromArray(array,size);
        cout<<req->type()<<endl;
        cout<<req->args()<<endl;
        cout<<req->opt()<<endl;
        zjy::response* res = new zjy::response;
        res->set_status(200);
        res->set_opt(0);

        ExampleSearchService *serviceHandler = new ExampleSearchService;

        ServerDone *done = new ServerDone(connfd,res);

        serviceHandler->Search(NULL,req,res,done);

		close(connfd);
	}

	close(listenfd);
}
