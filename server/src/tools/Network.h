#pragma once

#include <iostream>
#include <winsock2.h>
#include <pthread.h>
#include <queue>

struct ClientThread {
	int id;
	int active = false;
	int client;
	
	pthread_t thread;
	
	void client_thread_routine() {}
	
};

struct ClientThreadSpool {
	int size;
	std::vector<ClientThread*> threadPool;
	
	ClientThreadSpool(int numThreads) {
		this->size = numThreads;
	}
	
	ClientThread* getThread(int i) {
		if(i >= this->threadPool.size()) {
			std::cout << "ClientThreadSpool::getThread(int): Retrieving invalid object" << std::endl;
			return nullptr;
		}
		return this->threadPool.at(i);
	}
	
	bool full() {
		return this->activeThreads() == this->size;
	}
	
	int activeThreads() {
		int active = 0;
		for(int i=0;i<this->threadPool.size();i++) {
			if(this->threadPool.at(i)->active) active++;
		}
		return active;
	}
	
	bool initialize(void*(routine)(void*)) {
		for(int i=0;i<this->size;i++) {
			ClientThread *worker = new ClientThread();
			worker->id = i;
			if(pthread_create(&worker->thread,NULL,routine,worker) != 0) return false;
			this->threadPool.push_back(worker);
		}
		return true;
	}
	
};

struct Server {
    WSADATA WSAData;
 
    SOCKET server;
 
    SOCKADDR_IN serverAddr, clientAddr;
 
    int clientAddrSize = sizeof(clientAddr);
	
	std::queue<SOCKET> connections;
	
	void push_socket_queue(SOCKET);
	int pop_socket_queue();
	
	bool active = false;
	
	bool isFull();

	int TBA_accept();
	bool TBA_service(SOCKET);
	bool TBA_send(SOCKET,std::string);
	 
	void TBA_close(SOCKET);
	 
	void TBA_start();
	 
	void safe_print(const std::string&);
	 	
	Server(pthread_mutex_t lock) {
		WSAStartup(MAKEWORD(2,0), &this->WSAData);
		this->server = socket(AF_INET, SOCK_STREAM, 0);
	 
		this->serverAddr.sin_addr.s_addr = INADDR_ANY;
		this->serverAddr.sin_family = AF_INET;
		this->serverAddr.sin_port = htons(5555);
	}
	
	~Server() {
		std::cout << "Server closing" << std::endl;
	}
};