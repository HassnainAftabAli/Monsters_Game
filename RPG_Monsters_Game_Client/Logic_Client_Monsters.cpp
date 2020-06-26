#include "Logic_Client_Monsters.h"

using namespace std;

wchar_t Logic_Client_Monsters::screen[Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size];
HANDLE Logic_Client_Monsters::hConsole;
size_t screen_buffer_size;

Logic_Client_Monsters::~Logic_Client_Monsters() {
	delete[] screen;
}

void Logic_Client_Monsters::UpdateLocations(string locations, string delimiter, wchar_t print_char) {
	DWORD dwBytesWritten = 0;
	
	std::replace(std::begin(screen), std::begin(screen) + screen_buffer_size, print_char, L' ');

	size_t pos = 0;
	size_t index = 0;
	std::string token;
	int xPos, yPos;

	while ((pos = locations.find(delimiter)) != std::string::npos) {
		token = locations.substr(0, pos);
		//if its even, store it as the x location, otherwise store as y location and enter it to our screen
		if (index % 2 == 0) {
			xPos = stoi(token);
		}
		else {
			yPos = stoi(token);
			screen[(yPos * c_grid_size) + xPos] = print_char;
		}
		
		locations.erase(0, pos + delimiter.length());
		index += 1;
	}

	Logic_Client_Monsters::screen[screen_buffer_size - 1] = '\0';
	WriteConsoleOutputCharacter(hConsole, screen, screen_buffer_size, { 0,0 }, &dwBytesWritten);
}

//any msg send by server will call this function
void Logic_Client_Monsters::MessageReceived(std::string msg_received)
{
	char entity_char = msg_received[0];
	msg_received.erase(0, 1);
	Logic_Client_Monsters::UpdateLocations(msg_received, "#", (wchar_t)entity_char);
}


int Logic_Client_Monsters::JoinGame()
{
	screen_buffer_size = Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size;
	// Create Screen Buffer
	Logic_Client_Monsters::hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	
	//Setting Console Window Size
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, c_grid_size*8, c_grid_size*16, true); //since default font is consolas with char width 8 and height 16

	string inputstr;
	Client_TCP *client = new Client_TCP;

	if (client->Connect("127.0.0.1", 54000))
	{
		client->ListenRecvInThread(MessageReceived);

		while (true)
		{
		}
	}
	else {
		cout << "Connect() fail." << endl;
	}

	delete client;

	std::cin.get(); // wait
	return 0;
}