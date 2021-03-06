#include "SoftBodyScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
using namespace CommonUtils;

SoftBodyScene::SoftBodyScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
	, m_pPlayer(NULL)
{
}

SoftBodyScene::~SoftBodyScene()
{

}


void SoftBodyScene::OnInitializeScene()
{
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(true);

	//Set the camera position
	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(140.f);
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(-20.f);

	m_AccumTime = 0.0f;

	//Example usage of Log 
	//- The on screen log can be opened from the bottom left though the
	//  main point of this is the ability to output to file easily from anywhere.
	//  So if your having trouble debugging with hundreds of 'cout << vector3()' just
	//  through them into NCLLOG() and look at the 'program_output.txt' later =]
	NCLDebug::Log("This is a log entry - It will printed to the console, on screen log and <project_dir>\program_output.txt");
	//NCLERROR("THIS IS AN ERROR!"); // <- On Debug mode this will also trigger a breakpoint in your code!



	//<--- SCENE CREATION --->
	//Create Ground
	this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


	//Create Player (See OnUpdateScene)
	m_pPlayer = BuildCuboidObject(
		"Player1",					// Optional: Name
		Vector3(5.f, 0.5f, 0.0f),	// Position
		Vector3(0.5f, 0.5f, 1.0f),  // Half-Dimensions
		true,						// Physics Enabled?
		0.f,						// Physical Mass (must have physics enabled)
		true,						// Physically Collidable (has collision shape)
		false,						// Dragable by user?
		Vector4(0.1f, 0.1f, 0.1f, 1.0f)); // Render color
	this->AddGameObject(m_pPlayer);

	//Create SoftBody (See OnUpdateScene)


	GameObject* SoftBody[10][10];
	//make top two not move and 0 mass
	for (unsigned i = 0; i < 10; i++) {
		for (unsigned iq = 0; iq < 10; iq++) {

			if (iq == 9 && (i == 9 || i == 0)) {
				SoftBody[i][iq] = BuildSphereObject(
					"SoftBody",					// Optional: Name
					Vector3(i, iq, 0.0f),				// Position
					0.5f,			// Half-Dimensions
					true,				// Physics Enabled?
					0.0f,				// Physical Mass (must have physics enabled)
					true,				// Physically Collidable (has collision shape)
					false,				// Dragable by user?
					Vector4(4.0f, 8.0f, 4.0f, 1.0f));

				this->AddGameObject(SoftBody[i][iq]);

			}
			else {

				SoftBody[i][iq] = BuildSphereObject(
					"SoftBody",					// Optional: Name
					Vector3(i, iq, 0.0f),				// Position
					0.5f,			// Half-Dimensions
					true,				// Physics Enabled?
					1.0f,				// Physical Mass (must have physics enabled)
					true,				// Physically Collidable (has collision shape)
					true,				// Dragable by user?
					Vector4(4.0f, 8.0f, 4.0f, 1.0f));

				this->AddGameObject(SoftBody[i][iq]);

			}



		}
	}

	//forloop again adding constraints. 
	for (unsigned i = 0; i < 10; i++) {
		for (unsigned iq = 0; iq < 10; iq++) {
			if (i != 0) {
				PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(SoftBody[i][iq]->Physics(), SoftBody[i - 1][iq]->Physics(), SoftBody[i][iq]->Physics()->GetPosition(), SoftBody[i - 1][iq]->Physics()->GetPosition()));
			}
			if (iq != 0) {
				PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(SoftBody[i][iq]->Physics(), SoftBody[i][iq - 1]->Physics(), SoftBody[i][iq]->Physics()->GetPosition(), SoftBody[i][iq - 1]->Physics()->GetPosition()));


			}

		}
	}


	//Pendulum Constraints

	//PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(pendulumSphere->Physics(), pendulumSphereRoot->Physics(), pendulumSphere->Physics()->GetPosition(), pendulumSphereRoot->Physics()->GetPosition()));
}

void SoftBodyScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
}

void SoftBodyScene::OnUpdateScene(float dt)
{
	m_AccumTime += dt;

	// You can add status entries to the top left from anywhere in the program
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.2f, 0.2f, 1.0f), "Welcome to the Game Tech Framework!");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   You can move the black car with the arrow keys");

	// You can print text using 'printf' formatting
	bool donkeys = false;
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   The %s in this scene are dragable", donkeys ? "donkeys" : "cubes");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Left click to move");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Right click to rotate (They will be more spinnable after tutorial 2)");

	//Or move our car around the scene..
	{
		const float mv_speed = 5.f;				//Motion: Meters per second
		const float rot_speed = 90.f * dt;		//Rotation: Degrees per second

		bool boosted = false;

		PhysicsNode* pobj = m_pPlayer->Physics();
		Vector3 vel;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		{
			vel = pobj->GetOrientation().ToMatrix3()* Vector3(0.0f, 0.0f, -mv_speed);
			boosted = true;
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		{
			vel = pobj->GetOrientation().ToMatrix3()* Vector3(0.0f, 0.0f, mv_speed);
		}
		pobj->SetLinearVelocity(vel);
		pobj->SetPosition(pobj->GetPosition() + vel * dt);

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		{
			pobj->SetOrientation(pobj->GetOrientation() *
				Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), rot_speed));
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		{
			pobj->SetOrientation(pobj->GetOrientation() *
				Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), -rot_speed));
		}


		// If you ever need to see any visual output from your algorithms, for debugging or otherwise
		// you can now use NCLDebug:: from anywhere in the program using world space coordinates. The only
		// requirement is that it gets called each frame you need something renderered.
		//
		// All NCLDebug::Draw functions come in either normal or NDT (not depth tested) variants relating to if
		// you want them to clip with objects in the world or always be visible.
		if (boosted)
		{
			//Draw the rocket booster on the car using NCLDebug
			Vector3 backward_dir = pobj->GetOrientation().ToMatrix3() * Vector3(0, 0, 1);
			NCLDebug::DrawPoint(pobj->GetPosition() + backward_dir, 0.3f, Vector4(1.f, 0.7f, 0.0f, 1.0f));
			NCLDebug::DrawPoint(pobj->GetPosition() + backward_dir * 1.333f, 0.26f, Vector4(0.9f, 0.5f, 0.0f, 1.0f));
			NCLDebug::DrawPoint(pobj->GetPosition() + backward_dir * 1.666f, 0.23f, Vector4(0.8f, 0.3f, 0.0f, 1.0f));
			NCLDebug::DrawPoint(pobj->GetPosition() + backward_dir * 2.f, 0.2f, Vector4(0.7f, 0.2f, 0.0f, 1.0f));
		}
	}
}

