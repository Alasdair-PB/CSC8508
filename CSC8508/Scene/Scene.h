//
// Contributors: Alfie
//

#ifndef SCENE_H
#define SCENE_H

/**
 * Base class for all game scenes. Scenes should be saved on the heap, then registered using SceneManager::Set().
 */
class Scene {
public:
    virtual ~Scene() = default;

    /**
     * Called when the SceneManager has just changed the current Scene to this one.
     */
    virtual void OnLoad() { }

    /**
     * Called every tick when this Scene is the SceneManager's current scene.
     * @param dt Delta time: The change in time from the last tick in seconds
     */
    virtual void Update(float dt) { }

    /**
     * Called when the SceneManager has just changed the current scene away from this one.
     */
    virtual void OnUnload() { }
};

// SceneType must be a child class of Scene, not including Scene itself
template <typename T>
concept SceneType = std::is_base_of_v<Scene, T> && !std::is_same_v<Scene, T>;

#endif //SCENE_H
