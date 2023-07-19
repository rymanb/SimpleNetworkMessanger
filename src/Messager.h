#pragma once

#include "NetworkAPI.h"

class Messager
{
public:
	Messager(bool server);
	~Messager();
	
	void Init();
	void Update();
	
private:
	void CheckForMessages();
	
	Client mSelf;
	Network* mNetwork;

	std::string mReceiverName;
	std::string mMessage;

	bool mServer;
};