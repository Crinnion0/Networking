#pragma once

#include <ncltech\Scene.h>

class SoftBodyScene : public Scene
{
public:
	SoftBodyScene(const std::string& friendly_name);
	virtual ~SoftBodyScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;


protected:
	float m_AccumTime;
	GameObject* m_pPlayer;

	
};