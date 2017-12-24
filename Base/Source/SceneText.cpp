#include "SceneText.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "MeshBuilder.h"
#include "Application.h"
#include "Utility.h"
#include "LoadTGA.h"
#include <sstream>
#include "KeyboardController.h"
#include "MouseController.h"
#include "SceneManager.h"
#include "GraphicsManager.h"
#include "ShaderProgram.h"
#include "EntityManager.h"
#include "Particle.h"

#include "GenericEntity.h"
#include "GroundEntity.h"
#include "TextEntity.h"
#include "SpriteEntity.h"
#include "Light.h"
#include "SkyBox/SkyBoxEntity.h"
#include "SceneGraph\SceneGraph.h"
#include "SpatialPartition\SpatialPartition.h"

#include <iostream>
using namespace std;

SceneText* SceneText::sInstance = new SceneText(SceneManager::GetInstance());

SceneText::SceneText()
{
}

SceneText::SceneText(SceneManager* _sceneMgr)
{
	_sceneMgr->AddScene("Start", this);
}

SceneText::~SceneText()
{
	CSpatialPartition::GetInstance()->RemoveCamera();
	CSceneGraph::GetInstance()->Destroy();
}

void SceneText::Init()
{
	currProg = GraphicsManager::GetInstance()->LoadShader("default", "Shader//Texture.vertexshader", "Shader//MultiTexture.fragmentshader");

	MeshBuilder::GetInstance()->Init();
	
	// Tell the shader program to store these uniform locations
	{
		currProg->AddUniform("MVP");
		currProg->AddUniform("MV");
		currProg->AddUniform("MV_inverse_transpose");
		currProg->AddUniform("material.kAmbient");
		currProg->AddUniform("material.kDiffuse");
		currProg->AddUniform("material.kSpecular");
		currProg->AddUniform("material.kShininess");
		currProg->AddUniform("lightEnabled");
		currProg->AddUniform("numLights");
		currProg->AddUniform("lights[0].type");
		currProg->AddUniform("lights[0].position_cameraspace");
		currProg->AddUniform("lights[0].color");
		currProg->AddUniform("lights[0].power");
		currProg->AddUniform("lights[0].kC");
		currProg->AddUniform("lights[0].kL");
		currProg->AddUniform("lights[0].kQ");
		currProg->AddUniform("lights[0].spotDirection");
		currProg->AddUniform("lights[0].cosCutoff");
		currProg->AddUniform("lights[0].cosInner");
		currProg->AddUniform("lights[0].exponent");
		currProg->AddUniform("lights[1].type");
		currProg->AddUniform("lights[1].position_cameraspace");
		currProg->AddUniform("lights[1].color");
		currProg->AddUniform("lights[1].power");
		currProg->AddUniform("lights[1].kC");
		currProg->AddUniform("lights[1].kL");
		currProg->AddUniform("lights[1].kQ");
		currProg->AddUniform("lights[1].spotDirection");
		currProg->AddUniform("lights[1].cosCutoff");
		currProg->AddUniform("lights[1].cosInner");
		currProg->AddUniform("lights[1].exponent");
		currProg->AddUniform("colorTextureEnabled[0]");
		currProg->AddUniform("colorTextureEnabled[1]");
		currProg->AddUniform("colorTexture[0]");
		currProg->AddUniform("colorTexture[1]");
		currProg->AddUniform("textEnabled");
		currProg->AddUniform("textColor");
	}
	
	// Tell the graphics manager to use the shader we just loaded
	GraphicsManager::GetInstance()->SetActiveShader("default");

	// Light
	{
		lights[0] = new Light();
		GraphicsManager::GetInstance()->AddLight("lights[0]", lights[0]);
		lights[0]->type = Light::LIGHT_DIRECTIONAL;
		lights[0]->position.Set(0, 20, 0);
		lights[0]->color.Set(1, 1, 1);
		lights[0]->power = 1;
		lights[0]->kC = 1.f;
		lights[0]->kL = 0.01f;
		lights[0]->kQ = 0.001f;
		lights[0]->cosCutoff = cos(Math::DegreeToRadian(45));
		lights[0]->cosInner = cos(Math::DegreeToRadian(30));
		lights[0]->exponent = 3.f;
		lights[0]->spotDirection.Set(0.f, 1.f, 0.f);
		lights[0]->name = "lights[0]";

		lights[1] = new Light();
		GraphicsManager::GetInstance()->AddLight("lights[1]", lights[1]);
		lights[1]->type = Light::LIGHT_DIRECTIONAL;
		lights[1]->position.Set(1, 1, 0);
		lights[1]->color.Set(1, 1, 0.5f);
		lights[1]->power = 0.4f;
		lights[1]->name = "lights[1]";

		currProg->UpdateInt("numLights", 1);
		currProg->UpdateInt("textEnabled", 0);
	}
	
	// Create the playerinfo instance, which manages all information about the player
	playerInfo = CPlayerInfo::GetInstance();
	playerInfo->Init();

	// Create and attach the camera to the scene
	camera.Init(playerInfo->GetPos(), playerInfo->GetTarget(), playerInfo->GetUp());
	playerInfo->AttachCamera(&camera);
	GraphicsManager::GetInstance()->AttachCamera(&camera);

	// Set up the Spatial Partition and pass it to the EntityManager to manage
	{
		CSpatialPartition::GetInstance()->Init(100, 100, 10, 10);
		CSpatialPartition::GetInstance()->SetMesh("GRIDMESH");
		CSpatialPartition::GetInstance()->SetCamera(&camera);
		CSpatialPartition::GetInstance()->SetLevelOfDetails(40000.0f, 160000.0f);
		EntityManager::GetInstance()->SetSpatialPartition(CSpatialPartition::GetInstance());
	}

	/// Create entities into the scene
			GenericEntity* aCube = Create::Entity("cube", Vector3(-150, 0.0f, 150));
			aCube->SetCollider(true);
			aCube->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
			aCube->InitLOD("cube", "sphere", "cubeSG");
			
			// Add the pointer to this new entity to the Scene Graph
			CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(aCube);
			if (theNode == NULL)
			{
				cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
			}
	
	//
	//GenericEntity* anotherCube = Create::Entity("cube", Vector3(-20.0f, 1.1f, -20.0f));
	//anotherCube->SetCollider(true);
	//anotherCube->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
	//CSceneNode* anotherNode = theNode->AddChild(anotherCube);
	//if (anotherNode == NULL)
	//{
	//	cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
	//}
	//
	//GenericEntity* baseCube = Create::Asset("cube", Vector3(0.0f, 0.0f, 0.0f));
	//CSceneNode* baseNode = CSceneGraph::GetInstance()->AddNode(baseCube);
	//
	//CUpdateTransformation* baseMtx = new CUpdateTransformation();
	//baseMtx->ApplyUpdate(1.0f, 0.0f, 0.0f, 1.0f);
	//baseMtx->SetSteps(-60, 60);
	//baseNode->SetUpdateTransformation(baseMtx);
	//
	//GenericEntity* childCube = Create::Asset("cubeSG", Vector3(0.0f, 0.0f, 0.0f));
	//CSceneNode* childNode = baseNode->AddChild(childCube);
	//childNode->ApplyTranslate(0.0f, 1.0f, 0.0f);
	//
	//GenericEntity* grandchildCube = Create::Asset("cubeSG", Vector3(0.0f, 0.0f, 0.0f));
	//CSceneNode* grandchildNode = childNode->AddChild(grandchildCube);
	//grandchildNode->ApplyTranslate(0.0f, 0.0f, 1.0f);
	//CUpdateTransformation* aRotateMtx = new CUpdateTransformation();
	//aRotateMtx->ApplyUpdate(1.0f, 0.0f, 0.0f, 1.0f);
	//aRotateMtx->SetSteps(-120, 60);
	//grandchildNode->SetUpdateTransformation(aRotateMtx);
	
	//Create Gift entities
	{
		//Spiky
		GenericEntity* Spiky = Create::Entity("Spiky0", Vector3(-20.0f, 0.0f, -20.0f));
		Spiky->SetCollider(true);
		Spiky->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
		Spiky->InitLOD("Spiky0", "Spiky1", "Spiky2");
		
		// Add the pointer to this new entity to the Scene Graph
		CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(Spiky);
		if (theNode == NULL)
		{
			cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
		}
	}

	//Create House entities
	{
		//Beige
		GenericEntity* Beige = Create::Entity("Beige", Vector3(-20.0f, -10.f, -20.0f), Vector3(50, 50, 50));
		Beige->SetCollider(true);
		Beige->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
		
		// Add the pointer to this new entity to the Scene Graph
		CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(Beige);
		if (theNode == NULL)
		{
			cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
		}
	}
	
	SkyBoxEntity* theSkyBox = Create::SkyBox("SKYBOX_FRONT", "SKYBOX_BACK",
											 "SKYBOX_LEFT", "SKYBOX_RIGHT",
											 "SKYBOX_TOP", "SKYBOX_BOTTOM");

	// Customise the ground entity
	{
		groundEntity = Create::Ground("Snow", "Snow");
		groundEntity->SetPosition(Vector3(0, -10, 0));
		groundEntity->SetScale(Vector3(100.0f, 100.0f, 100.0f));
		groundEntity->SetGrids(Vector3(10.0f, 1.0f, 10.0f));
		playerInfo->SetTerrain(groundEntity);
	}

	/// Create a CEnemy instance
	//srand(time(NULL));
	//for (int i = 0; i < 10; i++)
	//{
	//	theEnemy = new CEnemy();
	//	float x = 1.0f + (i * rand() % 1000 - 500.0f);
	//	float y = 1.0f + (i * rand() % 1000 - 500.0f);
	//	theEnemy->SetRandomSeed(rand());
	//	theEnemy->Init(x, y);
	//	theEnemy->SetTerrain(groundEntity);
	//	theEnemy->SetTarget(theEnemy->GenerateTarget());
	//	theEnemy = NULL;
	//}

	// Setup the 2D entities
	{
		Create::Sprite2DObject("crosshair", Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f));
		invis = Create::Sprite2DObject("invis", Vector3(350.0f, -225.0f, 1.0f), Vector3(40.0f, 100.0f, 0.0f));
		Create::Sprite2DObject("powerbar", Vector3(350.0f, -225.0f, 0.0f), Vector3(25.0f, 100.0f, 0.0f));
		float halfWindowWidth = Application::GetInstance().GetWindowWidth() / 2.0f;
		float halfWindowHeight = Application::GetInstance().GetWindowHeight() / 2.0f;
		float fontSize = 25.0f;
		float halfFontSize = fontSize / 2.0f;
		for (int i = 0; i < 3; ++i)
		{
			textObj[i] = Create::Text2DObject("text", Vector3(-halfWindowWidth, -halfWindowHeight + fontSize*i + halfFontSize, 0.0f), "", Vector3(fontSize, fontSize,	fontSize), Color(0.0f,1.0f,0.0f));
		}
		textObj[0]->SetText("HELLO WORLD");
	}

	//Init Particle Stuff
	//m_particleCount = 0;
	//MAX_PARTICLE = 200;
}

void SceneText::Update(double dt)
{
	// Update our entities
	EntityManager::GetInstance()->Update(dt);

	// THIS WHOLE CHUNK TILL <THERE> CAN REMOVE INTO ENTITIES LOGIC! Or maybe into a scene function to keep the update clean
	if(KeyboardController::GetInstance()->IsKeyDown('1'))
		glEnable(GL_CULL_FACE);
	if(KeyboardController::GetInstance()->IsKeyDown('2'))
		glDisable(GL_CULL_FACE);
	if(KeyboardController::GetInstance()->IsKeyDown('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(KeyboardController::GetInstance()->IsKeyDown('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(KeyboardController::GetInstance()->IsKeyDown('5'))
	{
		lights[0]->type = Light::LIGHT_POINT;
	}
	else if(KeyboardController::GetInstance()->IsKeyDown('6'))
	{
		lights[0]->type = Light::LIGHT_DIRECTIONAL;
	}
	else if(KeyboardController::GetInstance()->IsKeyDown('7'))
	{
		lights[0]->type = Light::LIGHT_SPOT;
	}

	if(KeyboardController::GetInstance()->IsKeyDown('I'))
		lights[0]->position.z -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('K'))
		lights[0]->position.z += (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('J'))
		lights[0]->position.x -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('L'))
		lights[0]->position.x += (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('O'))
		lights[0]->position.y -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('P'))
		lights[0]->position.y += (float)(10.f * dt);

	if (KeyboardController::GetInstance()->IsKeyReleased('M'))
	{
		CSceneNode* theNode = CSceneGraph::GetInstance()->GetNode(1);
		Vector3 pos = theNode->GetEntity()->GetPosition();
		theNode->GetEntity()->SetPosition(Vector3(pos.x + 50.0f, pos.y, pos.z + 50.0f));
	}
	if (KeyboardController::GetInstance()->IsKeyReleased('N'))
	{
		CSpatialPartition::GetInstance()->PrintSelf();
	}

	// if the left mouse button was released
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::LMB))
	{
		cout << "Left Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::RMB))
	{
		cout << "Right Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::MMB))
	{
		cout << "Middle Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_XOFFSET) != 0.0)
	{
		cout << "Mouse Wheel has offset in X-axis of " << MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_XOFFSET) << endl;
	}
	if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) != 0.0)
	{
		cout << "Mouse Wheel has offset in Y-axis of " << MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) << endl;
	}
	// <THERE>

	// Update the player position and other details based on keyboard and mouse inputs
	playerInfo->Update(dt);

	invis->SetPosition(Vector3(invis->GetPosition().x, -225.f + playerInfo->GetMultiplier() * (invis->GetScale().y / 100), invis->GetPosition().z));

	//camera.Update(dt); // Can put the camera into an entity rather than here (Then we don't have to write this)

	GraphicsManager::GetInstance()->UpdateLights(dt);

	// Update the 2 text object values. NOTE: Can do this in their own class but i'm lazy to do it now :P
	// Eg. FPSRenderEntity or inside RenderUI for LightEntity
	std::ostringstream ss;
	ss.precision(5);
	float fps = (float)(1.f / dt);
	ss << "FPS: " << fps;
	textObj[1]->SetText(ss.str());

	std::ostringstream ss1;
	ss1.precision(4);
	ss1 << "Player:" << playerInfo->GetPos();
	textObj[2]->SetText(ss1.str());
}

void SceneText::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GraphicsManager::GetInstance()->UpdateLightUniforms();

	// Setup 3D pipeline then render 3D
	GraphicsManager::GetInstance()->SetPerspectiveProjection(45.0f, 4.0f / 3.0f, 0.1f, 10000.0f);
	GraphicsManager::GetInstance()->AttachCamera(&camera);
	EntityManager::GetInstance()->Render();

	// Setup 2D pipeline then render 2D
	int halfWindowWidth = Application::GetInstance().GetWindowWidth() / 2;
	int halfWindowHeight = Application::GetInstance().GetWindowHeight() / 2;
	GraphicsManager::GetInstance()->SetOrthographicProjection(-halfWindowWidth, halfWindowWidth, -halfWindowHeight, halfWindowHeight, -10, 10);
	GraphicsManager::GetInstance()->DetachCamera();
	EntityManager::GetInstance()->RenderUI();
}

void SceneText::Exit()
{
	// Detach camera from other entities
	GraphicsManager::GetInstance()->DetachCamera();
	playerInfo->DetachCamera();

	if (playerInfo->DropInstance() == false)
	{
#if _DEBUGMODE==1
		cout << "Unable to drop PlayerInfo class" << endl;
#endif
	}

	// Delete the lights
	delete lights[0];
	delete lights[1];
}
