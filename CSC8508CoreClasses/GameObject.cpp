//
// Contributors: Alasdair
//

#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8508;

GameObject::GameObject(const bool newIsStatic): isStatic(newIsStatic)	{
	worldID			= -1;
	isEnabled		= true;
	layerID = Layers::LayerID::Default;
	tag = Tags::Tag::Default;
	renderObject	= nullptr;
	networkObject	= nullptr;
	components = vector<IComponent*>();
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

GameObject::~GameObject()	{
	delete renderObject;
	delete networkObject;
}
