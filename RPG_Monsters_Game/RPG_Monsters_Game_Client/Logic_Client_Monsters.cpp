#include "Logic_Client_Monsters.h"

using namespace std;

wchar_t Logic_Client_Monsters::screen[Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size];
HANDLE Logic_Client_Monsters::hConsole;
size_t screen_buffer_size;
HWND console;
int threat_count = 0;

//connects the client to the server
int Logic_Client_Monsters::JoinGame(std::string ip, int port)
{
	screen_buffer_size = Logic_Client_Monsters::c_grid_size * Logic_Client_Monsters::c_grid_size;
	// Create Screen Buffer
	Logic_Client_Monsters::hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	//Setting Console Window Size
	console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, (c_grid_size + 2) * 8, (c_grid_size) * 16, true); //since default font is consolas with char width 8 and height 16

	string inputstr;
	this->client_instance = new Client_TCP;

	if (this->client_instance->Connect(ip, port))
	{
		this->client_instance->ListenRecvInThread(MessageReceived);

		while (true)
		{
			this->RecordPlayerMovement();
		}
	}
	else {
		cout << "Connect() fail." << endl;
	}

	delete this->client_instance;

	std::cin.get(); // wait
	return 0;
}

//any msg sent by server will call this function
void Logic_Client_Monsters::MessageReceived(std::string msg_received)
{
	if (msg_received.compare("m_nearby") == 0) {
		if (threat_count < 5) {
			threat_count++;
			swprintf_s(&screen[Logic_Client_Monsters::c_grid_size - 16], 16, L"THREAT COUNT: %d", threat_count);
		}
		else {
			swprintf_s(&screen[Logic_Client_Monsters::c_grid_size - 16], 16, L"    YOU LOST   ", threat_count);
		}
	}
	else {
		char entity_char = msg_received[0];
		msg_received.erase(0, 1);
		Logic_Client_Monsters::UpdateLocations(msg_received, "#", (wchar_t)entity_char);
	}
}

//updates the locations of the entities
void Logic_Client_Monsters::UpdateLocations(string locations, string delimiter, wchar_t print_char) {
	DWORD dwBytesWritten = 0;

	//empty past location
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

	//write onto the console
	WriteConsoleOutputCharacter(hConsole, screen, screen_buffer_size, { 0,0 }, &dwBytesWritten);
}

//to record if the player pressed a key
bool KeyPressed(char key) {
	if ((GetAsyncKeyState((unsigned short)key) & 0x8000) && console == GetForegroundWindow()) {
		//this is to avoid multiple movements on a single key-down
		while (GetAsyncKeyState((unsigned short)key) & 0x8000) {
		}
		return true;
	}
	return false;
}

//allows controlling the player using WSAD keys
void Logic_Client_Monsters::RecordPlayerMovement() {
	size_t steps = 1; //we are setting this to 1 to give it a game-like feel. Can be changed for other purposes or cin by user.
	if (KeyPressed('A')) {
		std::string command = "move_x -" + std::to_string(steps);
		//this needs to be placed inside each if statement as RecordPlayerMovement is in an infinite loop
		this->client_instance->Send(command);
	}
	else if (KeyPressed('D')) {
		std::string command = "move_x " + std::to_string(steps);
		this->client_instance->Send(command);
	}
	else if (KeyPressed('W')) {
		std::string command = "move_y -" + std::to_string(steps);
		this->client_instance->Send(command);
	}
	else if (KeyPressed('S')) {
		std::string command = "move_y " + std::to_string(steps);
		this->client_instance->Send(command);
	}
}

Logic_Client_Monsters::~Logic_Client_Monsters() {
	delete[] screen;
}

