
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "Scene_PathFinding.h"

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
float rotation = 0.0f;

int startX, startY, endX, endY;

void Win32_PrintAllAdapterIPAddresses();


struct returnNodes {

	int sx, sy, ex, ey;
};

int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}

int main(int arcg, char** argv)
{

	srand(98798797);

	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");


	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();

	MazeGenerator* generator = new MazeGenerator();
	initialiseData maze_initialise;
	returnNodes startend_initialise;
	SearchAStar* pathfinder = new SearchAStar();

	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("- New Client Connected\n");
				break;

			case ENET_EVENT_TYPE_RECEIVE:

				if (evnt.packet->dataLength == sizeof(initialiseData))
				{
					
					memcpy(&maze_initialise, evnt.packet->data, sizeof(initialiseData));

					printf("\t Maze Initialisation Data Recieved \n");
					generator->Generate(maze_initialise);
					

					string sendWallIndexs = generator->findWalls();

					const char* data = sendWallIndexs.c_str();
					//char* boop = "hi";

					cout << strlen(data);
					//const char* data = sendWallIndexs.c_str();


					ENetPacket* wallIndexPAcket = enet_packet_create(data, sizeof(char)*sendWallIndexs.length() , 0);
					enet_host_broadcast(server.m_pNetwork, 0, wallIndexPAcket);

				
					
				}

				if (evnt.packet->dataLength == sizeof(returnNodes)) {

					//Find start and end node

					memcpy(&startend_initialise, evnt.packet->data, sizeof(returnNodes));
					printf("\t Maze Start and End Nodes Data Recieved \n");

					generator->start = &generator->allNodes[startend_initialise.sx + startend_initialise.sy * maze_initialise.grid_size];
					generator->end = &generator->allNodes[startend_initialise.ex + startend_initialise.ey * maze_initialise.grid_size];
						

					//A star pathfinding to calculate the route through the maze.


					//Update A Star 
					//Equal weighting
					float weightingG = 1.0f;
					float weightingH = 1.0f;




					pathfinder->SetWeightings(weightingG, weightingH);

					GraphNode* start = generator->GetStartNode();
					GraphNode* end = generator->GetGoalNode();
					pathfinder->FindBestPath(start, end);
					
					

					list<const GraphNode*> completepath = pathfinder->GetFinalPath();
					
					string pathData;
					pathData += "#";

					for (list<const GraphNode*>::iterator i = completepath.begin(); i != completepath.end(); i++) {
						
						
						pathData += '|' + to_string((*i)->_pos.x) + ',' + to_string((*i)->_pos.y);


					}

					const char* sendNodes= pathData.c_str();
					

					cout << strlen(sendNodes);
					


					ENetPacket* nodepath_packet = enet_packet_create(sendNodes, sizeof(char)*pathData.length(), 0);
					enet_host_broadcast(server.m_pNetwork, 0, nodepath_packet);
					

				}
			
				if (evnt.packet->dataLength == sizeof(char[2])) {

					printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);

				}

				if (evnt.packet->data[0] == '@') {
					if (evnt.packet->data[1] == '5') {
						
						
									
						generator->Generate(maze_initialise);
						

						string sendWallIndexs = generator->findWalls();
						string callRegenerate = "@R";

						const char* data = sendWallIndexs.c_str();
						const char* callbackData = callRegenerate.c_str();
				

						ENetPacket* wallIndexPAcket = enet_packet_create(data, sizeof(char)*sendWallIndexs.length(), 0);
						enet_host_broadcast(server.m_pNetwork, 0, wallIndexPAcket);

						ENetPacket* callbackPacket = enet_packet_create(callbackData, sizeof(char)*callRegenerate.length(), 0);
						enet_host_broadcast(server.m_pNetwork, 0, callbackPacket);


						

					}


				}
				

				enet_packet_destroy(evnt.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		});
		
		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP)
		{

			//Packet data
			// - At the moment this is just a position update that rotates around the origin of the world
			//   though this can be any variable, structure or class you wish. Just remember that everything 
			//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)
			accum_time = 0.0f;
			Vector3 pos = Vector3(
				cos(rotation) * 2.0f,
				1.5f,
				sin(rotation) * 2.0f);

			ENetPacket* position_update1 = enet_packet_create(&generator,sizeof(MazeGenerator),0 );

			//Create the packet and broadcast it (unreliable transport) to all clients
			//ENetPacket* position_update = enet_packet_create(&pos, sizeof(Vector3), 0);
			//enet_host_broadcast(server.m_pNetwork, 0, position_update);
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
}




//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}

void UpdateAStarPreset()
{
	
	
}