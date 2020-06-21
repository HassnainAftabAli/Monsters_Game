#pragma once

#include <iostream>
#include <string>
#include "Client_TCP.h"

using namespace std;

class Logic_Client_Monsters {
private:
	static void MessageReceived(std::string msg_received);

public:
	int JoinGame();
};
