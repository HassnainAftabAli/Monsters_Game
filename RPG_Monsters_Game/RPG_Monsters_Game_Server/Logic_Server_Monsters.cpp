#include "Logic_Server_Monsters.h"

using namespace std;

bool monsters_active = false;

int Logic_Server_Monsters::StartGame(std::string ip, int port) {
	this->LoadServer(ip, port);

	return 0;
}

//initialize the tcp_ip server
void Logic_Server_Monsters::LoadServer(std::string ip, int port) {
	this->m_game_server = new Server_TCP(this, ip, port, this->MessageReceived, this->NewClientConnected);

	if (this->m_game_server->Init())
	{
		this->m_game_server->RunInThread();
		this->m_is_game_running = true;
		this->ServerCommandHandler(); //initilize the server command listener
	}
	else {
		cout << "Failed to init server." << endl;
	}
	delete this->m_game_server;  //also close all sockets
	cin.get();
}

//called everytime client sends a message
void Logic_Server_Monsters::MessageReceived(Logic_Server_Monsters* this_instance, SOCKET client, std::string msg) {
	this_instance->ClientCommandHandler(client, msg);
}

//called everytime a new client connects
void Logic_Server_Monsters::NewClientConnected(Logic_Server_Monsters* this_instance, SOCKET client) {
	this_instance->InitPlayer(client);

	std::string serialized_locs = this_instance->MapValuesToStringSerializer(this_instance->m_player_locations, 'P');
	this_instance->m_game_server->SendToAll(serialized_locs.c_str());
}

//handles commands entered from the console by the server
void Logic_Server_Monsters::ServerCommandHandler() {
	std::string command;
	while (true)
	{
		std::cout << "Enter Command: ";
		std::getline(std::cin, command);

		if (command == "exit") { //exit the program
			this->m_is_game_running = false;
			break;
		}
		else if (command == "create_monster_pack") { //create monsters
			if (!monsters_active) {
				monsters_active = true;
				this->InitMonsters();
			}

			//On an extended Monsters game, process more types of commands here. Better yet, use enum with a switch statement 
		}
		else {
			std::cout << "Response: Invalid Command" << std::endl;
		}
	}
}

//handles commands sent by the client
void Logic_Server_Monsters::ClientCommandHandler(SOCKET client, std::string command) {
	std::string command_type; //e.g. move commands, angle command etc
	std::string instruction; //instruction substring will contain info like steps to move

	std::string::size_type pos = command.find('_');
	if (pos != std::string::npos)
	{
		command_type = command.substr(0, pos);
		instruction = command.substr(pos + 1, command.length());

		if (command_type == "move") {
			this->MovePlayer(client, instruction);
		}
		else {
			//On an extended Monsters game, process more types of commands here. Better yet, use enum with a switch statement 
		}
	}
	else {
		cout << "Player issued an invalid command";
	}
}

//initializes monsters in a diagonal starting from 0,0
void Logic_Server_Monsters::InitMonsters() {
	for (int i = 0; i < c_num_monsters; i++) {
		for (int j = 0; j < 2; j++) {
			this->m_monster_locations[i][j] = i;
		}
	}
	std::cout << std::endl << "Response: Monsters have been created" << std::endl;

	this->MoveMonsters();
}

//moves the monsters in a random direction within the range 2
void Logic_Server_Monsters::MoveMonsters() {
	this->m_monsters_movement_thread = std::thread([&]() 
		{
		while (this->m_is_game_running) {
			std::this_thread::sleep_for(std::chrono::seconds(5));

			std::string serialized_locs = SimpleInt2DArrayToStringSerializer(this->m_monster_locations, this->c_num_monsters, 'M');
			//send monster locations to clients every 5 seconds
			this->m_game_server->SendToAll(serialized_locs.c_str());

			srand(time(NULL));

			for (int i = 0; i < c_num_monsters; i++)
			{
				//monster randomly moves 1 or 2 units (limit 2) in a random direction
				int potential_xLoc = this->m_monster_locations[i][0] + ((rand() % 5) - 2);
				int potential_yLoc = this->m_monster_locations[i][1] + ((rand() % 5) - 2);

				//only move monster if it is within the bounds of the grid and if there is no monster there already
				if ((!this->LocationOutOfBounds(potential_xLoc, potential_yLoc)) && 
					(!this->LocationHasMonster(potential_xLoc, potential_yLoc)))
				{
					if (this->LocationNearEntity(potential_xLoc, potential_yLoc, 'P')) {
						for (auto& entry : this->m_player_locations) {
							if (LocationNearEntity(entry.second.xLoc, entry.second.yLoc, 'M')) { //identify which player is nearby
								SOCKET client = entry.first;
								this->m_game_server->Send(client, "m_nearby"); //inform the player there is a monster nearby
							}
						}
					}
					//put a monster in that location
					this->m_monster_locations[i][0] = potential_xLoc;
					this->m_monster_locations[i][1] = potential_yLoc;
					this->m_grid[potential_xLoc][potential_yLoc] = 'M';
					//log onto server
					std::cout << std::endl << "Info: Monster " << (i+1) << " moved to '" 
						<< potential_xLoc << ", " << potential_yLoc << std::endl;
				}
			}
		}
	});
}

//initialize the player at a random location
void Logic_Server_Monsters::InitPlayer(SOCKET client) {
	this->m_num_players += 1;
	int xLoc, yLoc;
	srand(time(NULL));
	do {
		xLoc = rand() % 50;
		yLoc = rand() % 50;
	} while (LocationHasMonster(xLoc, yLoc) || LocationHasPlayer(xLoc,yLoc)); //to ensure initalization occurs at an appropriate location

	location player_loc = { xLoc, yLoc };
	m_player_locations[client] = player_loc;
}

//move the player based on the command received from the client
void Logic_Server_Monsters::MovePlayer(SOCKET client, std::string instruction) {
	int move_amount = stoi(instruction.substr(1, instruction.length()));
	bool valid_move = false;
	location* loc = &(this->m_player_locations[client]);

	if (instruction[0] == 'x') { //in x direction
		int potential_xLoc = (*loc).xLoc + move_amount;
		if (!this->LocationOutOfBounds(potential_xLoc, this->c_grid_size / 2)) {
			(*loc).xLoc += move_amount;
			valid_move = true;
		}
	}
	else if (instruction[0] == 'y') { //in y direction
		int potential_yLoc = (*loc).yLoc + move_amount;
		if (!this->LocationOutOfBounds(this->c_grid_size / 2, potential_yLoc)) {
			(*loc).yLoc += move_amount;
			valid_move = true;
		}
	}
	//ensure there is no monster where the player intends to move 
	if ((!this->LocationHasMonster(((*loc).xLoc), (*loc).yLoc)) && valid_move) {
		if (this->LocationNearEntity(((*loc).xLoc), ((*loc).yLoc), 'M')) {
			this->m_game_server->Send(client, "m_nearby"); //inform the client there is a monster nearby. Player loses 1 point.
		}
		std::string serialized_locs = MapValuesToStringSerializer(this->m_player_locations, 'P');
		this->m_grid[(*loc).xLoc][(*loc).yLoc] = 'P'; //place a player at the location
		this->m_game_server->SendToAll(serialized_locs.c_str()); //send the location to all players
		//log onto server
		cout << std::endl << "Player " << (std::distance(this->m_player_locations.begin(), this->m_player_locations.find(client)) +1)
			<< " moved to " << ((*loc).xLoc) << ", " << ((*loc).yLoc) << endl;
	}
}

bool Logic_Server_Monsters::LocationHasMonster(int x, int y) {
	if ((!this->LocationOutOfBounds(x, y)) && (this->m_grid[x][y] == 'M')) {
		return true;
	}
	return false;
}

bool Logic_Server_Monsters::LocationHasPlayer(int x, int y) {
	if ((!this->LocationOutOfBounds(x, y)) && (this->m_grid[x][y] == 'P'))
		return true;
	return false;
}

bool Logic_Server_Monsters::LocationOutOfBounds(int x, int y) {
	if ((x < 0) || (x > (this->c_grid_size-1)) || (y < 0) || (y > (this->c_grid_size-1)))
		return true;
	return false;
}

//identifies if the specified entity (player or monster) is nearby the specified location
bool Logic_Server_Monsters::LocationNearEntity(int x, int y, char check_entity) { 
	for (int hor = -1; hor < 2; hor++) {
		for (int ver = -1; ver < 2; ver++) {
			if (!LocationOutOfBounds(x + hor, y + ver)) {
				if (this->m_grid[x + hor][y + ver] == check_entity) {
					return true;
				}
			}
		}
	}
	return false;
}

//a custom serializer to convert array into a string for the client to process
std::string Logic_Server_Monsters::SimpleInt2DArrayToStringSerializer(int arr[][2], int arr_i_size, char id_char) {
	std::string str(1, id_char);
	for (int i = 0; i < arr_i_size; i++) {
		str += std::to_string(arr[i][0]) + '#' + std::to_string(arr[i][1]) + '#';
	}
	return str;
}

//a custom serializer to convert map into a string for the client to process
std::string Logic_Server_Monsters::MapValuesToStringSerializer(std::map<SOCKET, location> player_locations, char id_char) {
	std::string str(1,id_char);
	for (const auto& loc : player_locations) {
		str += std::to_string(loc.second.xLoc) + '#' + std::to_string(loc.second.yLoc) + '#';
	}
	return str;
}

//destructor
Logic_Server_Monsters::~Logic_Server_Monsters() {
	if (this->m_is_game_running) {
		this->m_is_game_running = false;
		this->m_monsters_movement_thread.join(); //wait for it to finish properly
	}

	delete[] this->m_grid;
	delete[] this->m_monster_locations;
	delete this->m_game_server;
}