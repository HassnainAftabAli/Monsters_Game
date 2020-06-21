#include "Logic_Client_Monsters.h"

int main() {

	//giving the server some time to start before we create players
	Logic_Client_Monsters* player_1 = new Logic_Client_Monsters;
	player_1->JoinGame();
}