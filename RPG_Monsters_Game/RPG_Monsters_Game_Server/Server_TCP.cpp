//Server_TCP.cpp

#include "Server_TCP.h"

Server_TCP::Server_TCP(Logic_Server_Monsters* connected_instance, std::string ipAddress, int port, MessageRecievedHandler mr_handler, NewClientConnectedHandler ncc_handler)
	: m_connected_instance(connected_instance), m_ipAddress(ipAddress), m_port(port), MessageReceived(mr_handler), NewClientConnected(ncc_handler)
{
	FD_ZERO(&master);
	m_run_thread_running = false;
}

Server_TCP::~Server_TCP() {
	//closing all sockets 
	while (master.fd_count > 0)
	{
		SOCKET sock = master.fd_array[0]; //get the socket number
		FD_CLR(sock, &master); //remove it from the master file list
		closesocket(sock); //close the socket
	}

	WSACleanup();

	//finishing running thread
	if (m_run_thread_running)
	{
		m_run_thread_running = false;  //stopping loop in ThreadRecv() 
		m_run_thread.join();		   //wait for it to finish properly
	}
}

//initialize winsock
bool Server_TCP::Init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	if (wsInit != 0)
	{
		std::cerr << "Can't start Winsock, Err #" << wsInit << std::endl;
		return false;
	}

	return wsInit == 0;
}

//send message to 1 client
bool Server_TCP::Send(SOCKET clientSocket, std::string message)
{
	if (!message.empty())
	{
		return send(clientSocket, message.c_str(), (int)(message.size() + 1), 0) != SOCKET_ERROR; //send message to specified client
	}
	return false;
}

//send message to all clients
void Server_TCP::SendToAll(std::string message)
{
	if (!message.empty())
	{
		{
			for (unsigned int i = 0; i < master.fd_count; i++)
			{

				SOCKET outSock = master.fd_array[i]; //selected client
				if (outSock != m_listening_sock)
				{
					send(outSock, message.c_str(), (int)(message.size() + 1), 0); //send message
				}
			}
		}
	}
}

//just a thread container for the run command
void Server_TCP::RunInThread()
{
	this->m_run_thread = std::thread([&]()
		{
			Run();
		});
}

//initializes the server
void Server_TCP::Run()
{
	m_listening_sock = CreateSocket(); // create a listening socket
	if (m_listening_sock == INVALID_SOCKET)
	{
		std::cerr << "Error in Run(), could not start server." << std::endl;
		return;
	}

	FD_SET(m_listening_sock, &master); //insert listening socket

	m_run_thread_running = true;
	while (m_run_thread_running)
	{
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr); //identify the communicating client

		//loop through all the current connections / potential connection
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			//is it an inbound communication?
			if (sock == m_listening_sock)
			{
				SOCKET client = accept(m_listening_sock, nullptr, nullptr); //accept a new connection
				FD_SET(client, &master); //add the new connection to the list of connected clients

				if (NewClientConnected != NULL) {
					NewClientConnected(this->m_connected_instance, client);
				}
			}
			else //it's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				//receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					//drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					//handle msg externally from class
					if (MessageReceived != NULL)
					{
						MessageReceived(this->m_connected_instance, sock, std::string(buf, 0, bytesIn));
					}
				}
			}
		}
	}
	
	FD_CLR(m_listening_sock, &master); //remove the listening socket from the master file descriptor set
	closesocket(m_listening_sock); //close it to prevent anyone else trying to connect.
	std::cout << "Server stopped" << std::endl;

	while (master.fd_count > 0)
	{
		SOCKET sock = master.fd_array[0]; // Get the socket number
		FD_CLR(sock, &master); // Remove it from the master file list
		closesocket(sock); //close socket
	}
}

SOCKET Server_TCP::CreateSocket()
{
	SOCKET sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_server != INVALID_SOCKET)
	{
		//fill in the hint structure
		m_hint.sin_family = AF_INET;
		m_hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &m_hint.sin_addr);

		int bindOk = bind(sock_server, (sockaddr*)&m_hint, sizeof(m_hint));// bind the ip address and port to a socket

		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(sock_server, SOMAXCONN); //tell Winsock this socket is for listening 
			if (listenOk == SOCKET_ERROR)
			{
				std::cerr << "Can't listen to socket, Err #" << WSAGetLastError() << std::endl;
				return SOCKET_ERROR;
			}
		}
		else {
			std::cerr << "Can't bind to socket, Err #" << WSAGetLastError() << std::endl;
			return SOCKET_ERROR;
		}

	}
	else {
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	return sock_server;
}
