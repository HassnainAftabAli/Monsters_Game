#include "Logic_Server_Monsters.h"

using namespace std;

void Logic_Server_Monsters::MessageReceived(Server_TCP* listen_server, SOCKET client, string msg)
{
	//send message to all other clients
	std::string strOut;
	strOut = strOut + "SOCKET #" + to_string(client) + ": " + msg + "\r";
	listen_server->SendToAllExcept(client, strOut);

	//logging msg in the server
	cout << "SOCKET " << client << ": " << msg << "\r";
}

void Logic_Server_Monsters::LoadServer(string ip, int port) {
	string inputstr;
	Server_TCP* server = new Server_TCP(ip, port, MessageReceived);

	if (server->Init())
	{
		server->RunInThread();

		while (true)
		{
			std::getline(std::cin, inputstr);
			if (inputstr == "exit") break;

			inputstr += "\n";
			//broadcasting input message (from server)
			server->SendToAllExcept(NULL, inputstr);
		}
	}
	else {
		cout << "Fail to init server." << endl;
	}

	delete server;  //also close all sockets

	cin.get();
}

void Logic_Server_Monsters::InitMonsters() {
	for (int i = 0; i < c_num_monsters; i++) {
		for (int j = 0; j < 2; j++) {
			m_monster_locations[i][j] = i;
		}
	}
}

int Logic_Server_Monsters::StartGame()
{
	Logic_Server_Monsters::InitMonsters();
	Logic_Server_Monsters::LoadServer("127.0.0.1", 54000);


	return 0;
}