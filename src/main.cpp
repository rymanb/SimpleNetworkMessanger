#include "NetworkAPI.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "Messager.h"

enum Mode
{
	Sender = 0,
	Receiver
};

int main()
{
	// are server or client?
	Mode mode;
	printf("Enter 0 for Client, 1 for Server: ");
	scanf_s("%d", &mode);

	// clear line
	std::cout << "\033[1A";
	std::cout << "\033[2K";

	// create messager
	Messager messager((bool)mode);
	messager.Init();

	// update loop
	messager.Update();
	
	return 0;
}