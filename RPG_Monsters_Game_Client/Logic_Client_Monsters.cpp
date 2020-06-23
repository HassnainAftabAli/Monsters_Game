#include "Logic_Client_Monsters.h"

using namespace std;

char Logic_Client_Monsters::grid[c_grid_size][c_grid_size];
int	 Logic_Client_Monsters::m_monster_locations[c_num_monsters][2] = { 0 };
wchar_t Logic_Client_Monsters::screen[] = { ' ' };
HANDLE Logic_Client_Monsters::hConsole;

Logic_Client_Monsters::~Logic_Client_Monsters() {
	delete screen;
	delete[] grid;
	delete[] m_monster_locations;
}

//A very basic deserializer to receive the locations string from server and insert it into our grid
//we first remove old locations, and then insert to new locations
//we are doing it this way so we don't have to initialize the whole array over and over again
void Logic_Client_Monsters::UpdateLocations(string locations, string delimiter) {
	//nullifying old locations
	for (int i = 0; i < Logic_Client_Monsters::c_num_monsters; i++) {
		grid[m_monster_locations[i][0]][m_monster_locations[i][1]] = NULL;
	}

	size_t pos = 0;
	size_t index = 0;
	std::string token;
	int xPos, yPos;
	int curr_index_x = -1;
	int curr_index_y = -1;

	while ((pos = locations.find(delimiter)) != std::string::npos) {
		token = locations.substr(0, pos);
		//if its even, store it as the x location, otherwise store as y location and enter it into our grid
		if (index % 2 == 0) {
			xPos = stoi(token);
			curr_index_x += 1;
			curr_index_y = 0;
		}
		else {
			yPos = stoi(token);
			Logic_Client_Monsters::grid[xPos][yPos] = 'X';
			curr_index_y += 1;
		}

		Logic_Client_Monsters::m_monster_locations[curr_index_x][curr_index_y] = stoi(token);

		locations.erase(0, pos + delimiter.length());
		index += 1;
	}
}

void Logic_Client_Monsters::DrawGrid(){

	DWORD dwBytesWritten = 0;

	/*for (int i = 0; i < Logic_Client_Monsters::c_grid_size; i++) {
		for (int j = 0; j < Logic_Client_Monsters::c_grid_size; j++) {
			if (grid[i][j] != NULL) {
				screen[(j * c_grid_size) + i] = 'X';
			}
			else {
				screen[(j * c_grid_size) + i] = ' ';
			}
		}
		cout << endl;
	}*/

	for (int i = 0; i < Logic_Client_Monsters::c_num_monsters; i++) {
		screen[(Logic_Client_Monsters::m_monster_locations[i][1]*c_grid_size) + Logic_Client_Monsters::m_monster_locations[i][0]] = 'X';
	}

	Logic_Client_Monsters::screen[Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size - 1] = '\0';
	WriteConsoleOutputCharacter(hConsole, screen, Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size, { 0,0 }, &dwBytesWritten);


}

//any msg send by server will call this function
void Logic_Client_Monsters::MessageReceived(std::string msg_received)
{
	Logic_Client_Monsters::UpdateLocations(msg_received, "-");
}

int Logic_Client_Monsters::JoinGame()
{
	
	// Create Screen Buffer
	Logic_Client_Monsters::hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);



	//Setting Console Window Size
	/*HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, c_grid_size, c_grid_size, TRUE)*/;

	string inputstr;
	Client_TCP *client = new Client_TCP;

	if (client->Connect("127.0.0.1", 54000))
	{
		client->ListenRecvInThread(MessageReceived);

		while (true)
		{
			Logic_Client_Monsters::DrawGrid();
			/*std::getline(std::cin, inputstr);

			if (inputstr == "exit") break;

			client->Send(inputstr);*/
		}
	}
	else {
		cout << "Connect() fail." << endl;
	}

	delete client;

	std::cin.get(); // wait
	return 0;
}

