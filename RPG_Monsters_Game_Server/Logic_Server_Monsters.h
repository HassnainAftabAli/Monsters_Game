#pragma once

#include <iostream>
#include <string>
#include "Server_TCP.h"
#include <list>
#include <map>

using namespace std;

class Logic_Server_Monsters;

class Logic_Server_Monsters {
private:
	struct location {
		int xLoc, yLoc;
	};
	
	Server_TCP*						m_game_server;
	thread							m_monsters_movement_thread;
	bool							m_is_game_running;

	
	const int						c_grid_size	= 50;
	char							grid[50][50]; //square grid

	int								m_num_players = 0;
	std::map<SOCKET, location>		m_player_locations;
	
	const int						c_num_monsters = 5;
	int								m_monster_locations[5][2]; //loc of monsters
	
	void							LoadServer(std::string ip, int port);

	void							InitMonsters();
	void							MoveMonsters();

	void							InitPlayer(SOCKET client_sock);
	void							MovePlayer(SOCKET client, string instruction);

	string							SimpleInt2DArrayToStringSerializer(int arr[][2], int size, char id_char);
	string							MapValuesToStringSerializer(std::map<SOCKET, location> player_locations, char id_char);

	static void						MessageReceived(Logic_Server_Monsters* this_instance, SOCKET client, string msg);
	static void						NewClientConnected(Logic_Server_Monsters* this_instance, SOCKET client);

	void							ServerCommandHandler();
	void							ClientCommandHandler(SOCKET client, string command);
	
public:
	int								StartGame();

	~Logic_Server_Monsters();
};