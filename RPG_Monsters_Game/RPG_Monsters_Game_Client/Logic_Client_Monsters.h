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
	static wchar_t				screen[c_grid_size * c_grid_size]; //square grid
	static HANDLE				hConsole;

	void RecordPlayerMovement(); //allows controlling the player using WSAD keys

	static void MessageReceived(std::string msg_received); //callback handler

	static void UpdateLocations(string locations, string delimiter, wchar_t print_char); //updates the locations of entities

public:
	int JoinGame(std::string ip, int port);
	~Logic_Client_Monsters();
};