#include "Logic_Server_Monsters.h"

int main() {
	
	Logic_Server_Monsters* game_room_1 = new Logic_Server_Monsters;
	game_room_1->StartGame("127.0.0.1", 54000);

	delete game_room_1;
}