#include "Octree.h"



Octree::Octree(Vector3 halfDims, Vector3 cubePos, int level)

{

	this->OTPos = cubePos;
	this->OThalfDims = halfDims;
	this->currentLevel = level;

}



Octree::~Octree()
{
	for (int i = 0; i < 8; i++) {

		delete children[i];
	}

}

void Octree::checkEntities()
{
	//if ;




}

void Octree::Build()
{

	if (entitiesList.size() > maximumEntities || currentLevel >= maxLevel) {
		return;
	}

	children[0] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[1] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[2] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[3] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[4] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[5] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[6] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	children[7] = new Octree(OThalfDims / 2, Vector3(OThalfDims.x + (OThalfDims.x / 2), OThalfDims.y + (OThalfDims.y / 2), OThalfDims.z + (OThalfDims.z / 2)), currentLevel + 1);
	//dicide/8

	//check which objects are in each region 
	//if in more than one stay here
	//if only one put into a child

	for (int i = 0; i < 8; i++) {


	}
	//Call build again


	//move center using half dim
	//triple nesty boi


}

void Octree::debugDraw()
{
}


