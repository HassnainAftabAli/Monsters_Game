#include "Logic_Server_Monsters.h"

using namespace std;

Logic_Server_Monsters::~Logic_Server_Monsters() {
	delete[] this->grid;
	delete[] this->m_monster_locations;
}

//A very basic serializer to convert our int array to string to be able to send it to players
string Simple_2D_int_Array_To_String_Serializer(int arr[][2], int arr_i_size) {
	string str = "";
	for (int i = 0; i < arr_i_size; i++) {
		str += to_string(arr[i][0]);
		str += '-';
		str += to_string(arr[i][1]);
		str += '-';
	}
	return str;
}

void Logic_Server_Monsters::MessageReceived(Server_TCP* listen_server, SOCKET client, string msg)
{
	//send message to all other clients
	std::string strOut;
	strOut = strOut + "SOCKET #" + to_string(client) + ": " + msg + "\r";
	listen_server->SendToAll(strOut);

	//logging msg in the server
	cout << "SOCKET " << client << ": " << msg << "\r";
}

void Logic_Server_Monsters::ServerCommandHandler() {
	string command;
	while (true)
	{
		std::cout << "Enter Command: ";
		std::getline(std::cin, command);

		if (command == "exit") {
			this->is_game_running = false;
			break;
		}
		if (command == "create_monster_pack") {
			this->InitMonsters();
			std::cout << "Response: Monsters have been created" << std::endl;
		}
		else {
			std::cout << "Response: Invalid Command" << std::endl;
		}
	}
}

void Logic_Server_Monsters::LoadServer(string ip, int port) {

	this->game_server = new Server_TCP(ip, port, MessageReceived);

	if (this->game_server->Init())
	{
		this->game_server->RunInThread();

		this->is_game_running = true;

		this->ServerCommandHandler();
	}
	else {
		cout << "Fail to init server." << endl;
	}

	delete this->game_server;  //also close all sockets

	cin.get();
}

void Logic_Server_Monsters::InitMonsters() {
	for (int i = 0; i < c_num_monsters; i++) {
		for (int j = 0; j < 2; j++) {
			this->m_monster_locations[i][j] = i;
		}
	}

	Logic_Server_Monsters::MoveMonsters();
}

void Logic_Server_Monsters::MoveMonsters() {
	this->m_monsters_movement_thread = std::thread([&]()
		{
			while (this->is_game_running) {
				std::this_thread::sleep_for(std::chrono::seconds(5));
				
				string serialized_locs = Simple_2D_int_Array_To_String_Serializer(this->m_monster_locations, this->c_num_monsters);
				//send monster locations to clients every 5 seconds
				this->game_server->SendToAll(serialized_locs.c_str());

				srand(time(NULL));

				for (int i = 0; i < c_num_monsters; i++) {
					//monster randomly moves 1 or 2 units (limit 2) in a random direction
					int potential_xLoc = this->m_monster_locations[i][0] + ((rand() % 5) - 2);
					int potential_yLoc = this->m_monster_locations[i][1] + ((rand() % 5) - 2);

					//only move monster if it is within the bounds of the grid
					//and if there is no monster there already
					if (potential_xLoc > 0 && potential_xLoc < this->c_grid_size &&
						potential_yLoc > 0 && potential_yLoc < this->c_grid_size &&
						this->grid[potential_xLoc][potential_yLoc] != 'X') 
					{
						//put a monster in that location
						this->m_monster_locations[i][0] = potential_xLoc;
						this->m_monster_locations[i][1] = potential_yLoc;
						this->grid[potential_xLoc][potential_yLoc] = 'X';
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