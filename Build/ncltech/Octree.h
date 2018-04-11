
#include "PhysicsNode.h"	
#pragma once
class Octree;




class Octree {
public:



	Octree(Vector3 halfDims, Vector3 cubePos, int level);
	~Octree();




	inline void setObjects(std::vector<PhysicsNode*> entitiesList) { this->entitiesList = entitiesList; }

	void checkEntities();

	void Build();

	void debugDraw();


protected:




	unsigned int maximumEntities = 20;

	Vector3 OThalfDims; 
	Vector3 OTPos;
	std::vector<PhysicsNode*> entitiesList;
	Octree* children[8];

	int maxLevel;
	int currentLevel;
};

