#pragma once

#include <iostream>
#include "Server_TCP.h"
#include <map>

class Logic_Server_Monsters;

class Logic_Server_Monsters {
private:
	struct location {
		int xLoc, yLoc;
	};
									//----Game Environment----
	const size_t					c_grid_size = 50;
	char							m_grid[50][50]; //square grid
	bool							m_is_game_running;
	Server_TCP*						m_game_server;
	std::thread						m_monsters_movement_thread; //will contain timer to move monsters
	void							LoadServer(std::string ip, int port);

									//----Callback handlers----
	static void						MessageReceived(Logic_Server_Monsters* this_instance, SOCKET client, std::string msg);
	static void						NewClientConnected(Logic_Server_Monsters* this_instance, SOCKET client);

									//----Commands Handlers----
	void							ServerCommandHandler();
	void							ClientCommandHandler(SOCKET client, std::string command);

									//----Monster Related-----
	const size_t					c_num_monsters = 5;
	int								m_monster_locations[5][2]; //loc of monsters
	void							InitMonsters();
	void							MoveMonsters();

									//----Player Related----
	size_t							m_num_players = 0;
	std::map<SOCKET, location>		m_player_locations;
	void							InitPlayer(SOCKET client_sock);
	void							MovePlayer(SOCKET client, std::string instruction);
	void							KillPlayer(SOCKET client);

									//----Potential-location based checks----
	bool							LocationHasMonster(int x, int y);
	bool							LocationHasPlayer(int x, int y);
	bool							LocationOutOfBounds(int x, int y);
	bool							LocationNearMonster(int x, int y);

									//----Custom Serializers----
	std::string						SimpleInt2DArrayToStringSerializer(int arr[][2], int size, char id_char);
	std::string						MapValuesToStringSerializer(std::map<SOCKET, location> player_locations, char id_char);

public:
	int								StartGame();

	~Logic_Server_Monsters();
};