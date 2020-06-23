#pragma once

#include <iostream>
#include <string>
#include "Server_TCP.h"
#include <list>

using namespace std;

class Logic_Server_Monsters {
private:
	struct player_info {
		string id;		 //Identifying by IP
		int xLoc, yLoc;  //Coordinates on Grid
	};

	Server_TCP*						game_server;
	thread							m_monsters_movement_thread;
	bool							is_game_running;

	static const int				c_num_monsters = 5;
	static const int				c_grid_size	= 50;
	int								m_num_players = 0;
	
	int								m_monster_locations[c_num_monsters][2]; //loc of monsters
	list<player_info>				m_player_locations; //stores id of client, xloc, yloc
	char							grid[c_grid_size][c_grid_size]; //square grid
	
	void							LoadServer(std::string ip, int port);
	void							InitMonsters();
	void							MoveMonsters();

	static void						MessageReceived(Server_TCP* listen_server, SOCKET client, string msg);
	void							ClientCommandHandler(string command);

	void							ServerCommandHandler();
	
public:
	int								StartGame();

	~Logic_Server_Monsters();
};