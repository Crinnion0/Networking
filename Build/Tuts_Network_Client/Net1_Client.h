
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "MazeRenderer.h"
#include <nclgl\OBJMesh.h>

struct nodesData {
	int sx, sy, ex, ey;


};

//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;

	virtual void regenerateScene();

	virtual void renderMaze(initialiseData grid);

	void ProcessNetworkEvent(const ENetEvent& evnt);


	void setStartEnd() {


		cout << "Input Start Node X: (Integer)";
		cin >> this->startX;
		cout << "Input Start Node Y: (Integer)";
		cin >> this->startY;

		cout << "Input End Node X: (Integer)";
		cin >> this->endX;

		cout << "Input End Node Y: (Integer)";
		cin >> this->endY;

	}

	


protected:
	GameObject* box;

	NetworkBase network;
	ENetPeer*	serverConnection;

	MazeGenerator generator;
	MazeRenderer* renderer;
	initialiseData packet_data;


	SearchHistory searchHistory;

	vector<GraphNode*> completepath;

	GraphNode *start, *end;

	int startX, startY, endX, endY;
};