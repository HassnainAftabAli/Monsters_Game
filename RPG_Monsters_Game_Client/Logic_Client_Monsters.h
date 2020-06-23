#pragma once

#include <iostream>
#include <string>
#include "Client_TCP.h"

using namespace std;

class Logic_Client_Monsters {
private:
	static const int			c_num_monsters = 5;
	static const int			c_grid_size = 50;
	
	static char 				grid[c_grid_size][c_grid_size]; //square grid
	static int					m_monster_locations[c_num_monsters][2]; //loc of monsters

	static wchar_t				screen[c_grid_size * c_grid_size];
	static HANDLE				hConsole;

	static void MessageReceived(std::string msg_received);
	static void UpdateLocations(string locations, string delimiter);
	static void DrawGrid();

public:
	int JoinGame();
	~Logic_Client_Monsters();
};