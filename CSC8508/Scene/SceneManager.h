//
// Contributors: Alfie
//

#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H
#include "Scene.h"

/**
 * SceneManager is a static class that holds pointers to the game's different scenes and dictates the current. The game
 * loop should grab the current scene from here and call its Update().
 */
class SceneManager final {
public:
    /**
     * Changes the current Scene pointer to the target Scene and calls OnLoad() and OnUnload() in the new and old
     * current Scenes respectively.
     * @param scene Target Scene
     */
    static inline void SetCurrent(Scene* scene);

    /**
     * @return Current Scene pointer or NULLPTR if not yet set.
     */
    static Scene* GetCurrent() { return current; }

    /**
     * Sets the static singleton instance for a target Scene type.
     * @tparam S Scene type (child class)
     * @param scene Scene instance
     */
    template <typename S>
    static void SetInstance(S* const scene) { sceneInstance<S> = scene; }

    /**
     * Gets the static singleton instance for a target Scene type.
     * @tparam S Scene type (child class)
     * @return Scene instance or NULLPTR if not set
     */
    template <typename S>
    static S* GetInstance() { return sceneInstance<S>; }

    /**
     * Sets the current Scene pointer to the static singleton instance of a target scene type.
     * @tparam S
     */
    template <typename S>
    static void SetCurrentToInstance() { SetCurrent(sceneInstance<S>); }

private:
    static Scene* current;

    /**
     * Static singleton Scene instance: When making a new Scene type, a pointer to the scene should be stored here.
     * @tparam S
     */
    template <typename S>
    static S* sceneInstance;
};


template <typename S>
S* SceneManager::sceneInstance = nullptr;


inline void SceneManager::SetCurrent(Scene* scene) {
    Scene* old = current;
    current = scene;
    if (old) old->OnUnload();
    if (current) current->OnLoad();
}

#endif //SCENEMANAGER_H
