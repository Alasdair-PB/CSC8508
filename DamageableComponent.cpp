#ifndef DAMAGEABLE_COMPONENT_H
#define DAMAGEABLE_COMPONENT_H

#include <algorithm>
#include <iostream>

class DamageableComponent {
private:
    int health;
    int maxHealth;

public:
   
    DamageableComponent(int initialHealth, int initialMaxHealth)
        : health(std::max(0, initialHealth)), maxHealth(std::max(1, initialMaxHealth)) {
        health = std::min(health, maxHealth); 
    }

   
    void DamageComponent(int damage) {
        if (damage > 0) {
            health = std::max(0, health - damage);
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

    
    void SetHealth(int newHealth) {
        if (newHealth >= 0) {
            health = std::min(newHealth, maxHealth);
        }
    }

    
    int GetHealth() const {
        return health;
    }

    
    int GetMaxHealth() const {
        return maxHealth;
    }
};

#endif // DAMAGEABLE_COMPONENT_H
