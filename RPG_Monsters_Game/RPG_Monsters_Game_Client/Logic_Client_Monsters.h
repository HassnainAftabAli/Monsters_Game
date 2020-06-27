#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include "Client_TCP.h"

using namespace std;

class Logic_Client_Monsters {
private:
	Client_TCP					*client_instance;

	static const int			c_grid_size = 50;
	static wchar_t				screen[c_grid_size * c_grid_size];
	static HANDLE				hConsole;

	void RecordPlayerMovement();

	static void MessageReceived(std::string msg_received);
	static void UpdateLocations(string locations, string delimiter, wchar_t print_char);

public:
	int JoinGame();
	~Logic_Client_Monsters();
};