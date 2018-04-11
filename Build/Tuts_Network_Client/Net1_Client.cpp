/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!"
if it successfully connects. The server, will continually broadcast a packet containing a
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>

const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);




Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, box(NULL)
{
}

void Net1_Client::OnInitializeScene()
{
	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}

	//Generate Simple Scene with a box that can be updated upon recieving server packets
	box = CommonUtils::BuildCuboidObject(
		"Server",
		Vector3(0.0f, -0.1f, 0.0f),
		Vector3(2.7f, 0.1f, 2.7f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));
	this->AddGameObject(box);

	//wallmesh = new OBJMesh(MESHDIR"cube.obj");




}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	box = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);


	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
		regenerateScene();

	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	/*NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
		"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);*/


	if (completepath.size() > 0) {

		float grid_scalar = 1.0f / (float)packet_data.grid_size;
		Matrix4 transform = renderer->Render()->GetWorldTransform();
		for (unsigned i = 0; i < completepath.size() - 1; i++) {

			NCLDebug::DrawThickLine(Vector3(transform * Vector3((completepath[i]->_pos.x + 0.5f) * grid_scalar, 0.1f, (completepath[i]->_pos.y + 0.5f) * grid_scalar)),
				Vector3(transform * Vector3((completepath[i + 1]->_pos.x + 0.5f) * grid_scalar, 0.1f, (completepath[i + 1]->_pos.y + 0.5f) * grid_scalar)),
				0.1f,
				Vector4(1.0f, 0.0f, 0.0f, 1.0f));



		}
	}

	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
}

void Net1_Client::regenerateScene()
{

	char* regenerate = "@5";
	ENetPacket* regenPacket = enet_packet_create(regenerate, strlen(regenerate), 0);
	enet_peer_send(serverConnection, 0, regenPacket);




}

void Net1_Client::renderMaze(initialiseData grid)
{

	Mesh* wallmesh = new OBJMesh(MESHDIR"cube.obj");

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallmesh->SetTexture(whitetex);


	if (renderer) delete renderer;
	renderer = new MazeRenderer(&generator, wallmesh);
	Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f, 5.0f / float(grid.grid_size), 5.f)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));

	renderer->Render()->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)) * maze_scalar);

	this->AddGameObject(renderer);
}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{


	
	packet_data.grid_size = 10;
	packet_data.maze_density = 1.0f;
	nodesData packet_nodes;
	packet_nodes.ex, packet_nodes.ey = 0, packet_nodes.sx = 0, packet_nodes.sy = 0;
	string s;



	list<SearchHistoryElement> SearchHistory;


	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == serverConnection)
		{
			NCLDebug::Log(status_color3, "Network: Successfully connected to server!");



			setStartEnd();
			packet_nodes.sx = this->startX;
			packet_nodes.sy = this->startY;
			packet_nodes.ex = this->endX;
			packet_nodes.ey = this->endY;


			ENetPacket* mazePacket = enet_packet_create(&packet_data, sizeof(packet_data), 0);
			enet_peer_send(serverConnection, 0, mazePacket);

			//Send Nodes 
			ENetPacket* nodesPacket = enet_packet_create(&packet_nodes, sizeof(packet_nodes), 0);
			enet_peer_send(serverConnection, 0, nodesPacket);

			////Send a 'hello' packet
			char* text_data = "Test packet please ignore";
			ENetPacket* packet = enet_packet_create(text_data, strlen(text_data), 0);
			enet_peer_send(serverConnection, 0, packet);





		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{



		//Handling wall index packet
		if (evnt.packet->data[0] == '@') {

			if (evnt.packet->data[1] == 'R') {

				//packet_nodes = setStartEnd(packet_nodes);

				completepath.clear();

				setStartEnd();
				packet_nodes.sx = this->startX;
				packet_nodes.sy = this->startY;
				packet_nodes.ex = this->endX;
				packet_nodes.ey = this->endY;

				

				ENetPacket* mazePacket = enet_packet_create(&packet_data, sizeof(packet_data), 0);
				enet_peer_send(serverConnection, 0, mazePacket);

				ENetPacket* nodesPacket = enet_packet_create(&packet_nodes, sizeof(packet_nodes), 0);
				enet_peer_send(serverConnection, 0, nodesPacket);




			}





			//string s;



			for (unsigned i = 0; i < evnt.packet->dataLength; ++i) {

				if (evnt.packet->data[i] != '@') {

					s += evnt.packet->data[i];
				}

			}






			packet_data.maze_density = 0.0f;
			generator.Generate(packet_data);

			generator.start = &generator.allNodes[this->startX + this->startY* packet_data.grid_size];
			generator.end = &generator.allNodes[this->endX + this->endY * packet_data.grid_size];



			if (evnt.packet->data[1] = !'|') {




			}
			//pass in each number from s to edges
			for (unsigned i = 0; i < s.size(); ++i) {
				if (s[i] != '/') {
					if (s[i] == '1') {
						generator.allEdges[i / 2]._iswall = true;

					}
					if (s[i] == '0') {
						generator.allEdges[i / 2]._iswall = false;

					}

				}
			}



			renderMaze(packet_data);
			s.clear();

		}
		//Deal with incoming start and end node packets
		else if (evnt.packet->data[0] == '#') {

			int pointX, pointY;

			for (unsigned i = 1; i < evnt.packet->dataLength; i++) {



			while (i < evnt.packet->dataLength)
			{
				if (evnt.packet->data[i] == '|')
				{
					++i;
					string nodesPath = "";
					while (evnt.packet->data[i] != ',') {
						nodesPath += evnt.packet->data[i];
						++i;
					}
					++i;
					pointX = stoi(nodesPath);
					nodesPath = "";
					while (evnt.packet->data[i] != '|') {
						nodesPath += evnt.packet->data[i];
						++i;
					}
					pointY = stoi(nodesPath);
					nodesPath = "";

					completepath.push_back(&generator.allNodes[pointX + pointY * packet_data.grid_size]); // create vector of visited coords for navigation.
				}
				
				
			}

				}

			

		}



		else
		{
			NCLERROR("Recieved Invalid Network Packet!");
		}

	}
	break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		NCLDebug::Log(status_color3, "Network: Server has disconnected!");
	}
	break;
	}
}




