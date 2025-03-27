set(PROJECT_NAME CSC8508CoreClasses)

################################################################################
# Source groups
################################################################################
set(AI_Behaviour_Tree
    "BehaviourAction.h"
    "BehaviourNode.h"
    "BehaviourNodeWithChildren.h"
    "BehaviourSelector.h"
    "BehaviourSelector.cpp"
    "BehaviourSequence.h"
    "BehaviourSequence.cpp"
)
source_group("AI\\Behaviour Trees" FILES ${AI_Behaviour_Tree})

set(AI_Pushdown_Automata
    "PushdownMachine.h"
    "PushdownMachine.cpp"
    "PushdownState.h"
    "PushdownState.cpp"
)
source_group("AI\\Pushdown Automata" FILES ${AI_Pushdown_Automata})

set(AI_State_Machine
    "State.h"
    "StateMachine.h"  
    "StateMachine.cpp"
    "StateMachine.h"
    "StateTransition.h"
    "IState.h"
    "IStateTransition.h"
    "WaitState.h"
    "JumpState.h"
)
source_group("AI\\State Machine" FILES ${AI_State_Machine})

set(AI_Pathfinding
    "NavigationGrid.h"
    "NavigationGrid.cpp"  
    "NavigationMesh.cpp"
    "NavigationMesh.h"
    "NavigationMap.h"
    "NavigationPath.h"
)
source_group("AI\\Pathfinding" FILES ${AI_Pathfinding})


set(Collision_Detection
    "AABBVolume.h"
    "CapsuleVolume.h"  
    "CapsuleVolume.cpp"
    "CollisionDetection.h"
    "CollisionDetection.cpp"
	"CollisionEvent.h"
    "OBBVolume.h"
    "QuadTree.h"
    "QuadTree.cpp"
    "Ray.h"
    "SphereVolume.h"
)
source_group("Collision Detection" FILES ${Collision_Detection})

set(Networking
    "GameClient.h"  
    "GameClient.cpp"
    "GameServer.h"
    "GameServer.cpp"
    "NetworkBase.h"
    "NetworkBase.cpp"
    "NetworkObject.h"
    "NetworkObject.cpp"
    "NetworkState.h"
    "NetworkState.cpp"
)
source_group("Networking" FILES ${Networking})

set(Physics
    "constraint.h"  
    "constraint.h"  
    "PositionConstraint.cpp"
    "PositionConstraint.h"
    "OrientationConstraint.cpp"
    "OrientationConstraint.h"
    "PhysicsObject.cpp"
    "PhysicsObject.h"
    "PhysicsSystem.cpp"
    "PhysicsSystem.h"
)
source_group("Physics" FILES ${Physics})

set(Header_Files
    "Debug.h"
    "GameObject.h"
    "GameWorld.h"
    "RenderObject.h"
    "MaterialManager.h"
    "Transform.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "Debug.cpp"
    "GameObject.cpp"
    "GameWorld.cpp"
    "RenderObject.cpp"
    "Transform.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ComponentSystems
    "IComponent.h"
    "INetworkComponent.h"
    "INetworkDeltaComponent.h"
    "ComponentManager.h"
    "IComponent.cpp"
    "INetworkComponent.cpp"    
    "INetworkDeltaComponent.cpp"
)
source_group("ComponentSystems" FILES ${ComponentSystems})

set(Components
    "InputComponent.h"
    "CameraComponent.h"
    "PhysicsComponent.h"
    "BoundsComponent.h"
    "PhysicsComponent.cpp"
    "BoundsComponent.cpp"
    "StateComponent.cpp"
    "StateComponent.h"
    "StaminaComponent.h"
    "StaminaComponent.cpp"
    "AnimationComponent.cpp"
    "AnimationComponent.h"
    "SightComponent.h"
    "SightComponent.cpp"
    "FallDamageComponent.h"
    "FallDamageComponent.cpp"
    "TimerComponent.h"
)
source_group("Components" FILES ${Components})

set(NetworkComponents
    "InputNetworkComponent.h"
    "TransformNetworkComponent.h"
    "FullTransformNetworkComponent.h"
    "TimerNetworkComponent.h"
    "GameNetworkedManagerComponent.h"
)
source_group("NetworkComponents" FILES ${NetworkComponents})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${AI_Behaviour_Tree}
    ${AI_Pushdown_Automata}
    ${AI_State_Machine}
    ${AI_Pathfinding}
    ${Collision_Detection}
    ${Networking}
    ${Physics}
    ${Components}
    ${ComponentSystems}
    ${NetworkComponents}
)

set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

#set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS c;C;M;c++;cc;cpp;cxx;m;mm;mpp;CPP;ixx;cppm)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8508CoreClasses)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################

#[[
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "UNICODE"
    "_UNICODE"
    "WIN32_LEAN_AND_MEAN"
    "_WINSOCK_DEPRECATED_NO_WARNINGS"
	"NOMINMAX"
)
]]

target_precompile_headers(${PROJECT_NAME} PRIVATE
	<fstream>
    <sstream>
	<iostream>
	<iosfwd>
	
    <memory>
	<string>
	<thread>
    <filesystem>
    <functional>
	<assert.h>  
	<chrono>	
  
    <unordered_set>
	<set>
    <vector>
	<stack>
	<unordered_map>
    <map> 
	<list>
	<thread>
    <atomic>
	<functional>
	<stack> 
	
    "../NCLCoreClasses/Vector.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
    "../NCLCoreClasses/GameTimer.h"

)

################################################################################
# Compile and link options
################################################################################


################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "NCLCoreClasses"
    "ws2_32.lib"
    "Event"
)
add_subdirectory(enet)

include_directories("../NCLCoreClasses/")
include_directories("./")
include_directories("../Event/")
include_directories("../ImGui/")
include_directories("../SerializedSave/")

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32.lib")
endif()    

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Event)
