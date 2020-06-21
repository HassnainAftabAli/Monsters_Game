#include "Logic_Client_Monsters.h"

using namespace std;

//any msg send by server will call this function
void Logic_Client_Monsters::MessageReceived(std::string msg_received)
{
	std::cout << "MSG FROM SERVER> " << msg_received << endl;
}

int Logic_Client_Monsters::JoinGame()
{
	string inputstr;
	Client_TCP *client = new Client_TCP;

	cout << "Gets here";
	if (client->Connect("127.0.0.1", 54000))
	{
		client->ListenRecvInThread(MessageReceived);

		while (true)
		{
			std::getline(std::cin, inputstr);
			if (inputstr == "exit") break;

			client->Send(inputstr);
		}
	}
	else {
		cout << "Connect() fail." << endl;
	}

	delete client;

	cin.get(); // wait
	return 0;
}
