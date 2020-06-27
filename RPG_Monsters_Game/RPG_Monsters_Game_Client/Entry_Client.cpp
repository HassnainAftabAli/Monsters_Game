#include "Logic_Client_Monsters.h"

int main() {

	Logic_Client_Monsters* player_1 = new Logic_Client_Monsters;
	player_1->JoinGame("127.0.0.1", 54000);

	delete player_1;
}