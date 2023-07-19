#pragma once
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>


struct Socket;

struct Client
{
	Socket* socket;
	std::string name;
	
	Client* receiver = nullptr;
};

class Network
{
public:

	Network();
	~Network();

	// initialize the network API
	static bool Init();
	// shutdown the network API
	static void Shutdown();

	virtual void Update(std::function<void(void)> callback);
	
	// connect to a server
	virtual bool Connect(const char* ip, unsigned short port);
	
	// disconnect from the server
	virtual void Disconnect();
	
	// send data to the server
	virtual void Send(const char* data, int size);
	
	// receive data from the server
	virtual int Receive(char* data, int size);
	
	// check if the client is connected to a server
	bool IsConnected();
	
	
	// get the port of the server
	unsigned short GetPort();
	
	static std::string GetIP();

	
protected:
	Socket* mSocket;
};

class Server : public Network
{
public:	
	virtual bool Connect(const char* ip, unsigned short port);
	
	virtual void Update(std::function<void(void)> callback);
	
	virtual void Send(const char* data, int size) override;
	
	virtual int Receive(char* data, int size) override;
	
	virtual void Disconnect() override;
	


private:
	std::unordered_map<std::string, Client> mClients;
	
	
};

