#include "Messager.h"
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#endif

Messager::Messager(bool server) : mServer(server)
{
	// create network or server
	if (server)
	{
		mNetwork = new Server();
	}
	else
	{
		mNetwork = new Network();
		
		// get receiver name
		std::string name;
		printf("Enter your name: ");
		std::cin >> name;
		
		mSelf.name = name;
		
		// clear console
		std::cout << "\033[1A";
		std::cout << "\033[2K";
		


	}


	
}

Messager::~Messager()
{
	// shutdown network
	delete mNetwork;

	Network::Shutdown();
}

void Messager::Init()
{
	// initialize network 
	Network::Init();
	
	
	char ip[256]; // ip address of the server
	unsigned short port = 80; // port to connect to
	
	std::string localIP = "Local";
	


	if (!mServer)
	{
		std::cout << "Enter IP or Local: ";
		std::cin >> localIP;
	}
		
		
	if (localIP == "Local")
	{
		localIP = Network::GetIP();
	}

	
	// connect to server
	mNetwork->Connect(localIP.c_str(), port);
	
	// send name to server
	if (!mServer)
	{
		mNetwork->Send(mSelf.name.c_str(), mSelf.name.size() + 1);
	}	
}

void Messager::Update()
{
	while (true)
	{
		// update network
		mNetwork->Update([] {});
		CheckForMessages(); // check for messages

		// check for received messages
		char buffer[256];
		int size = mNetwork->Receive(buffer, sizeof(buffer));
		if (size > 0)
		{
			// clear the line
			std::cout << "\33[2K\r";

			std::string msg = buffer;
			
			// set color to red
			std::cout << "\033[1;31m";
			std::cout << msg << std::endl;
			std::cout << "\033[0m";

			// print the message
			if (mReceiverName == "")
			{
				std::cout << "To @server: ";
			}
			else
			{
				std::cout << "To @" << mReceiverName << ": ";
			}

			std::cout << mMessage;
		}
	}
}

void Messager::CheckForMessages()
{
#ifdef _WIN32

	if (_kbhit() != 0)
	{
		char c = _getch();
#else
	// Linux
	if (true)
	{
		char c = getchar();
#endif

		if (c == '\r')
		{
			// clear line 
			std::cout << "\33[2K\r";
			
			std::string formattedMessage = "<Self>" + mSelf.name + ": " + mMessage;
			
			// set color to green
			std::cout << "\033[32m";
			std::cout << formattedMessage << std::endl;
			std::cout << "\033[0m";


			// send message to server
			if (mMessage != "/Recipiant")
			{
				// check if the message is going to a specific client
				if (mReceiverName != "")
					mMessage = "<Receiver>" + mReceiverName + ": " + mMessage;

				// send
				mNetwork->Send(mMessage.c_str(), mMessage.size() + 1);
				mMessage.clear();
			}
			else
			{
				// change the recipiant
				std::cout << "Enter a valid recipiant" << std::endl;
				mNetwork->Send("<Clients>", 9); // request list of clients
				
				mMessage.clear();
				
				char buffer[256];
				
				int size = 0;

				// wait for response
				while (size <= 0)
				{
					size = mNetwork->Receive(buffer, sizeof(buffer));
				}
				
				// print the list of clients
				std::string msg = buffer;
				std::cout << msg << std::endl;
				
				// get the name of the receiver
				std::cin >> mReceiverName;
				
				// if the receiver is the server, clear the name
				if (mReceiverName == "everyone" || mReceiverName == "server")
				{
					mReceiverName = "";
				}
				
				// clear last 3 lines
				for (int i = 0; i < 3; ++i)
				{
					std::cout << "\033[1A";
					std::cout << "\033[2K";
				}

				// set color to blue
				std::cout << "\033[1;34m";
				std::cout << "Selected Recipiant: " << mReceiverName << std::endl;
				std::cout << "\033[0m";
			}

			// print the message
			if (mReceiverName == "")
			{
				std::cout << "To @server: ";
			}
			else
			{
				std::cout << "To @" << mReceiverName << ": ";
			}
		}
		else if (c == '\b')
		{
			// remove last character
			if (mMessage.size() > 0)
			{
				mMessage.pop_back();
				std::cout << "\b \b";
			}
		}
		else
		{
			// add character to message and print it
			mMessage += c;
			std::cout << c;
		}
	}

	
	
}


