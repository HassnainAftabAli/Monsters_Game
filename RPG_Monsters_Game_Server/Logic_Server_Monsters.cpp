#include "Logic_Server_Monsters.h"

using namespace std;

Logic_Server_Monsters::~Logic_Server_Monsters() {
	if (this->m_is_game_running) {
		this->m_is_game_running = false;
		this->m_monsters_movement_thread.join(); //wait for it to finish properly
	}

	delete[] this->grid;
	delete[] this->m_monster_locations;
	delete this->m_game_server;
}

string Logic_Server_Monsters::SimpleInt2DArrayToStringSerializer(int arr[][2], int arr_i_size, char id_char) {
	std::string str(1, id_char);
	for (int i = 0; i < arr_i_size; i++) {
		str += to_string(arr[i][0]);
		str += '#';
		str += to_string(arr[i][1]);
		str += '#';
	}
	return str;
}

string Logic_Server_Monsters::MapValuesToStringSerializer(std::map<SOCKET, location> player_locations, char id_char) {
	std::string str(1,id_char);
	for (const auto& loc : player_locations) {
		str += to_string(loc.second.xLoc);
		str += '#';
		str += to_string(loc.second.yLoc);
		str += '#';
	}
	return str;
}

void Logic_Server_Monsters::InitPlayer(SOCKET client_sock) {
	this->m_num_players += 1;

	srand(time(NULL));
	int xLoc = rand() % 50;
	int yLoc = rand() % 50;
	location player_loc = { xLoc, yLoc };

	m_player_locations[client_sock] = player_loc;
}

void Logic_Server_Monsters::MovePlayer(SOCKET client, string instruction) {
	if (instruction[0] == 'x')
		this->m_player_locations[client].xLoc += stoi(instruction.substr(1, instruction.length));
	else if (instruction[0] == 'y')
		this->m_player_locations[client].yLoc += stoi(instruction.substr(1, instruction.length));

	string serialized_locs = MapValuesToStringSerializer(this->m_player_locations, 'P');
	this->m_game_server->SendToAll(serialized_locs.c_str());
}

void Logic_Server_Monsters::NewClientConnected(Logic_Server_Monsters* this_instance, SOCKET client) {
	this_instance->InitPlayer(client);

	string serialized_locs = this_instance->MapValuesToStringSerializer(this_instance->m_player_locations, 'P');
	this_instance->m_game_server->SendToAll(serialized_locs.c_str());
}

void Logic_Server_Monsters::MessageReceived(Logic_Server_Monsters* this_instance, SOCKET client, string msg)
{
	this_instance->ClientCommandHandler(client, msg);
}

void Logic_Server_Monsters::ServerCommandHandler() {
	string command;
	while (true)
	{
		std::cout << "Enter Command: ";
		std::getline(std::cin, command);

		if (command == "exit") {
			this->m_is_game_running = false;
			break;
		}
		if (command == "create_monster_pack") {
			this->InitMonsters();
			std::cout << "Response: Monsters have been created" << std::endl;

			/*On an extended Monsters game, process more types of commands here.
			Better yet, if the number of commands increase considerably, use enum along with a switch statement.
			Place commands and mappings in a separate 'Commands' file.*/
		}
		else {
			std::cout << "Response: Invalid Command" << std::endl;
		}
	}
}

void Logic_Server_Monsters::ClientCommandHandler(SOCKET client, string command) {
	std::string command_type;
	std::string instruction;

	std::string::size_type pos = command.find('_');
	if (pos != std::string::npos)
	{
		command_type = command.substr(0, pos);
		instruction = command.substr(pos + 1, command.length());

		if (command_type == "move") {
			this->MovePlayer(client, instruction);
		} 
		else {
			/*On an extended Monsters game, process more types of commands here.
			Better yet, if the number of commands increase considerably, use enum along with a switch statement.
			Place commands and mappings in a separate 'Commands' file.*/
		}
	}
	else
	{
		cout << "Player issued an invalid command";
	}
}

void Logic_Server_Monsters::LoadServer(string ip, int port) {

	this->m_game_server = new Server_TCP(this, ip, port, this->MessageReceived, this->NewClientConnected);

	if (this->m_game_server->Init())
	{
		this->m_game_server->RunInThread();

		this->m_is_game_running = true;

		this->ServerCommandHandler();
	}
	else {
		cout << "Fail to init server." << endl;
	}

	delete this->m_game_server;  //also close all sockets

	cin.get();
}

void Logic_Server_Monsters::InitMonsters() {
	for (int i = 0; i < c_num_monsters; i++) {
		for (int j = 0; j < 2; j++) {
			this->m_monster_locations[i][j] = i;
		}
	}

	this->MoveMonsters();
}

void Logic_Server_Monsters::MoveMonsters() {
	this->m_monsters_movement_thread = std::thread([&]()
		{
			while (this->m_is_game_running) {
				std::this_thread::sleep_for(std::chrono::seconds(5));
				
				string serialized_locs = SimpleInt2DArrayToStringSerializer(this->m_monster_locations, this->c_num_monsters, 'M');
				//send monster locations to clients every 5 seconds
				this->m_game_server->SendToAll(serialized_locs.c_str());

				srand(time(NULL));

				for (int i = 0; i < c_num_monsters; i++) {
					//monster randomly moves 1 or 2 units (limit 2) in a random direction
					int potential_xLoc = this->m_monster_locations[i][0] + ((rand() % 5) - 2);
					int potential_yLoc = this->m_monster_locations[i][1] + ((rand() % 5) - 2);

					//only move monster if it is within the bounds of the grid
					//and if there is no monster there already
					if (potential_xLoc > 0 && potential_xLoc < this->c_grid_size &&
						potential_yLoc > 0 && potential_yLoc < this->c_grid_size &&
						this->grid[potential_xLoc][potential_yLoc] != 'M') 
					{
						//put a monster in that location
						this->m_monster_locations[i][0] = potential_xLoc;
						this->m_monster_locations[i][1] = potential_yLoc;
						this->grid[potential_xLoc][potential_yLoc] = 'M';
					}
				}
			}
		});
}

int Logic_Server_Monsters::StartGame()
{
	this->LoadServer("127.0.0.1", 54000);

	return 0;
}