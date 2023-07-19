#ifdef _WIN32 

#include "NetworkAPI.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <assert.h>
#include <string>

#include <future>
#include <thread>
#include <chrono>
#include <iostream>
#include <conio.h>

// the socket that is used to communicate with the server
//SOCKET sock = INVALID_SOCKET;

struct Socket
{
	SOCKET sock;
	char* ip;
	u_short port;
};

Network::Network()
{
	mSocket = new Socket();
	mSocket->sock = INVALID_SOCKET;

}

Network::~Network()
{
	delete mSocket;
}


bool Network::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		assert("Failed to initialize WinSock2\n");
		return false;
	}
	
	return true;
		
}

void Network::Shutdown()
{
	if (WSACleanup() != 0)
	{
		assert("Failed to shutdown WinSock2\n");
	}
}
static std::string getInput()
{
	std::string in;
	std::cin >> in;
	return in;
}


void Network::Update(std::function<void(void)> callback)
{


	
	callback();
}

bool Network::Connect(const char* ip, unsigned short port)
{
	// create a socket in non-blocking mode
	mSocket->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket->sock == INVALID_SOCKET)
	{
		assert("Failed to create socket\n");
		return false;
	}
	
	// set the socket to non-blocking mode
	u_long mode = 1;
	if (ioctlsocket(mSocket->sock, FIONBIO, &mode) != 0)
	{
		assert("Failed to set socket to non-blocking mode\n");
		return false;
	}

	HOSTENT* hostEntry;

	hostEntry = gethostbyname(ip);
	

	if (hostEntry == NULL)
	{
		assert("Failed to get host\n");
		return false;
	}
	
	// connect to the server
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = *((in_addr*)*hostEntry->h_addr_list);
	serverAddr.sin_port = htons(port);
	if (connect(mSocket->sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
	{
		assert("Failed to connect to server\n");
		return false;
	}
	
	return true;
}

void Network::Disconnect()
{
	if (mSocket->sock != INVALID_SOCKET)
	{
		std::string msg = "disconnect";
		send(mSocket->sock, msg.c_str(), msg.size() + 1, 0);

		
		closesocket(mSocket->sock);
		mSocket->sock = INVALID_SOCKET;
	}
}

void Network::Send(const char* data, int size)
{
	if (mSocket->sock != INVALID_SOCKET)
	{
		send(mSocket->sock, data, size, 0);
	}
}

int Network::Receive(char* data, int size)
{
	if (mSocket->sock != INVALID_SOCKET)
	{
		return recv(mSocket->sock, data, size, 0);
	}

	return 0;
}

bool Network::IsConnected()
{
	return mSocket->sock != INVALID_SOCKET;
}


unsigned short Network::GetPort()
{
	if (mSocket->sock != INVALID_SOCKET)
	{
		sockaddr_in addr;
		int len = sizeof(addr);
		if (getpeername(mSocket->sock, (sockaddr*)&addr, &len) == 0)
		{
			return ntohs(addr.sin_port);
		}
	}

	return 0;
}

bool Server::Connect(const char* ip, unsigned short port)
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	
	// create a socket
	mSocket->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// bind the socket to the port
	if (bind(mSocket->sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
	{
		assert("Failed to bind socket\n");
		return false;
	}

	// listen for connections
	if (listen(mSocket->sock, 1) != 0)
	{
		assert("Failed to listen for connections\n");
		return false;
	}

	u_long mode = 1;
	if (ioctlsocket(mSocket->sock, FIONBIO, &mode) != 0)
	{
		assert("Failed to set socket to non-blocking mode\n");
		return false;
	}

	return true;
}

void Server::Update(std::function<void(void)> callback)
{
	// check if there is a client trying to connect
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(mSocket->sock, &readSet);
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	if (select(0, &readSet, NULL, NULL, &timeout) > 0)
	{
		// accept the connection
		sockaddr_in clientAddr;
		int len = sizeof(clientAddr);
		SOCKET clientSock = accept(mSocket->sock, (sockaddr*)&clientAddr, &len);
		Socket* client = nullptr;
		if (clientSock != INVALID_SOCKET)
		{
			// add the client to the list
			client = new Socket();
			client->sock = clientSock;
			client->ip = inet_ntoa(clientAddr.sin_addr);
			client->port = ntohs(clientAddr.sin_port);
			//mClients.push_back(client);
		}
		else
		{
			assert("Failed to accept connection\n");
		}
		
		printf("Client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		// receive the client's name and send back a welcome message
		char name[256];
		recv(client->sock, name, sizeof(name), 0);
		std::string msg = "Welcome to the server, ";
		msg += name;
		send(client->sock, msg.c_str(), msg.size() + 1, 0);

		std::string nameS(name);

		Client c;
		c.socket = client;
		c.name = nameS;
		

		mClients[nameS] = c;
		
		
		// call the callback
		callback();
	}
}

void Server::Send(const char* data, int size)
{
	for (auto client : mClients)
	{
		send(client.second.socket->sock, data, size, 0);
	}
}

int Server::Receive(char* data, int size)
{
	for (auto client : mClients)
	{
		int bytes = recv(client.second.socket->sock, data, size, 0);
		if (bytes > 0)
		{
			std::string msg = data;

			if (msg == "disconnect")
			{
				printf("Client disconnected: %s:%d\n", client.second.socket->ip, client.second.socket->port);
				
				closesocket(client.second.socket->sock);

				Socket* clientToRemove = client.second.socket;
				
				mClients.erase(client.first);
				
				delete clientToRemove;
				
			}
			if (msg.find("<Receiver>") != std::string::npos)
			{
				// example: <Receiver>name: message

				// get the receiver's name
				std::string receiver = msg.substr(10, msg.find(":") - 10);
				
				// check if the receiver exists
				if (mClients.find(receiver) != mClients.end())
				{
					// get message after tag and name
					std::string message = msg.substr(msg.find(":") + 1, msg.size() - msg.find(":") - 1);
					

					std::string finalMessage = "<Sender>" + client.second.name + ": " + message;
					// send the message to the receiver
					send(mClients[receiver].socket->sock, finalMessage.c_str(), finalMessage.size() + 1, 0);
					
				}
				
			}
			else if(msg.find("<Clients>") != std::string::npos)
			{
				std::string clients = "<Clients>";
				for (auto client : mClients)
				{
					clients += client.first + ",";
				}
				send(client.second.socket->sock, clients.c_str(), clients.size() + 1, 0);
			}
			else
			{
				std::string finalMessage = "<Server>: " + msg;
				// send the message to all clients
				for (auto client : mClients)
				{
					send(client.second.socket->sock, finalMessage.c_str(), finalMessage.size() + 1, 0);
				}

			}
		}
	}
	
	return 0;
}

void Server::Disconnect()
{
	for (auto client : mClients)
	{
		closesocket(client.second.socket->sock);
		delete client.second.socket;
	}

	mClients.clear();

	if (mSocket->sock != INVALID_SOCKET)
	{
		closesocket(mSocket->sock);
		mSocket->sock = INVALID_SOCKET;
	}
}

// server


std::string Network::GetIP()
{
	// return the local IP address
	char hostName[256];
	gethostname(hostName, sizeof(hostName));
	hostent* host = gethostbyname(hostName);
	if (host != nullptr)
	{
		in_addr** addrs = (in_addr**)host->h_addr_list;
		for (int i = 0; addrs[i] != nullptr; i++)
		{
			return inet_ntoa(*addrs[i]);
		}
	}
}



#endif
