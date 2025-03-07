#ifndef DAMAGEABLE_COMPONENT_H
#define DAMAGEABLE_COMPONENT_H

#include <algorithm>  
#include <iostream>
#include <functional> 
#include "IComponent.h" 
#include "Event.h" 
#include "EventManager.h" 
#include "GameObject.h" 


class DeathEvent : public Event {
private:
    GameObject& gameObject;

public:
    DeathEvent(GameObject& obj) : gameObject(obj) {}
    GameObject& GetGameObject() { return gameObject; }
};

class DamageableComponent : public IComponent {
private:
    int health;
    int maxHealth;
    GameObject& owner; 

public:
    
    DamageableComponent(GameObject& gameObject, int initialHealth, int initialMaxHealth)
        : IComponent(), owner(gameObject),
        health(std::max(0, initialHealth)),
        maxHealth(std::max(1, initialMaxHealth)) {
        health = std::min(health, maxHealth); 
    }

   
    void DamageComponent(int damage) {
        if (damage > 0) {
            health = std::max(0, health - damage);
            if (health <= 0) {
                InvokeDeathEvent();
            }
        }
    }

    
    void HealComponent(int healthRegain) {
        if (healthRegain > 0) {
            health = std::min(maxHealth, health + healthRegain);
        }
    }

    
    bool IsDestroyed() const {
        return health == 0;
    }

    
    void SetMaxHealth(int newMaxHealth) {
        if (newMaxHealth > 0) {
            maxHealth = newMaxHealth;
            health = std::min(health, maxHealth); 
        }
    }

    
   

   
    int GetHealth() const {
        return health;
    }

    
    int GetMaxHealth() const {
        return maxHealth;
    }

private:
    
    void InvokeDeathEvent() {
        auto event = DeathEvent(owner);
        EventManager::Call(&event);
    }
};

#endif // DAMAGEABLE_COMPONENT_H
