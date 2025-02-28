#include "../NCLCoreClasses/KeyboardMouseController.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "Legacy/MainMenu.h"
#include "Math.h"
#include "Legacy/UpdateObject.h"


#pragma once
#include "GameTechRenderer.h"
#include "UISystem.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "Legacy/PlayerGameObject.h"
#include "BoundsComponent.h"
#include <vector>
using std::vector;


namespace NCL {
	namespace CSC8508 {

		struct NetworkSpawnData
		{
			int objId;
			int ownId;
			bool clientOwned;
		};


		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();

			void SetPause(bool state);
			void InitWorld();
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void UpdateObjectSelectMode(float dt);
			bool SelectObject();
			void MoveSelectedObject();
			void LockedObjectMovement();
      
			void DrawUIElements();
      
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddNavMeshToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);

			void EndGame(bool hasWon);

			void UpdateScore(float points);

			bool RayCastNavWorld(Ray& r, float rayDistance);
			void UpdateDrawScreen(float dt);
			bool OnEndGame(float dt);

			void  CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation);
			std::vector<Vector3>  GetVertices(Mesh* navigationMesh, int i);

			MainMenu* GetMainMenu() { return mainMenu; }


#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inPause = false;
			bool inSelectionMode;

			float forceMagnitude;
			float time = 0;
			int score = 0;

			BoundsComponent* selectionObject = nullptr;

			Mesh* navigationMesh = nullptr;
			NavigationPath outPath;
			NavigationMesh* navMesh = nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			MainMenu* mainMenu = nullptr;

			BoundsComponent* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);

			void LockCameraToObject(BoundsComponent* o) {
				lockedObject = o;
			}


			std::vector<Vector4> colors = {
				Vector4(1.0f, 0.0f, 0.0f, 1.0f), // Red
				Vector4(0.0f, 1.0f, 0.0f, 1.0f), // Green
				Vector4(0.0f, 0.0f, 1.0f, 1.0f), // Blue
				Vector4(1.0f, 1.0f, 0.0f, 1.0f), // Yellow
				Vector4(1.0f, 0.0f, 1.0f, 1.0f), // Magenta
				Vector4(0.0f, 1.0f, 1.0f, 1.0f)  // Cyan
			};

			GameObject* objClosest = nullptr;
      
			UISystem* uiSystem;
			float framerateDelay = 0;
			float latestFramerate;
		};
	}
}

