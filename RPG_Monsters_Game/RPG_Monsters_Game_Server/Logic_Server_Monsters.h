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
									//called everytime server receives a message
	static void						MessageReceived(Logic_Server_Monsters* this_instance, SOCKET client, std::string msg); 
									//called everytime a new client connects
	static void						NewClientConnected(Logic_Server_Monsters* this_instance, SOCKET client);

									//----Commands Handlers----
	void							ServerCommandHandler(); //handles commands entered in server console
	void							ClientCommandHandler(SOCKET client, std::string command); //handles commands sent by connected clients

									//----Monster Related-----
	const size_t					c_num_monsters = 5;
	int								m_monster_locations[5][2]; //loc of monsters
	void							InitMonsters(); //initializes the monsters
	void							MoveMonsters(); //randomly moves monsters within a range

									//----Player Related----
	size_t							m_num_players = 0;
	std::map<SOCKET, location>		m_player_locations;
	void							InitPlayer(SOCKET client_sock);  //initializes the player
	void							MovePlayer(SOCKET client, std::string instruction); //moves the player based on player's command

									//----Potential-location based checks----
	bool							LocationHasMonster(int x, int y);
	bool							LocationHasPlayer(int x, int y);
	bool							LocationOutOfBounds(int x, int y);
	bool							LocationNearEntity(int x, int y, char check_entity);

									//----Custom Serializers----
	std::string						SimpleInt2DArrayToStringSerializer(int arr[][2], int size, char id_char);
	std::string						MapValuesToStringSerializer(std::map<SOCKET, location> player_locations, char id_char);

public:
	int								StartGame(std::string ip, int port);

	~Logic_Server_Monsters();
};