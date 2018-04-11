#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>

//Fully striped back scene to use as a template for new scenes.
class Scene2 : public Scene
{
public:
	Scene2(const std::string& friendly_name);

	virtual ~Scene2();
	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;


protected:
	float m_AccumTime;
	GameObject* m_pPlayer;

};

