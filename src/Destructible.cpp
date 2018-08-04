#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float hp, float defense, const char *corpseName):
    maxHp(maxHp), hp(hp), defense(defense), corpseName(corpseName) {
    }

float Destructible::heal(float amount) {
    hp += amount;
    if (hp > maxHp) {
        amount -= hp-maxHp;
        hp = maxHp;
    }
    return amount;
}


float Destructible::takeDamage(Actor *owner, float damage) {
    damage -= defense;
    if (damage > 0) {
        hp -= damage;
        if (hp <= 0) {
            die(owner);
        }
    } else {
        damage = 0;
    }
    return damage;
}

void Destructible::die(Actor *owner) {
    // transform actor into corpse
    owner->ch = '%';
    owner->col = TCODColor::darkRed;
    owner->name = corpseName;
    owner->blocks = false;
    // make sure corpses are drawn before living actors
    engine.sendToBack(owner);
}

MonsterDestructible::MonsterDestructible(float maxHp, float hp, float defense, 
    const char *corpseName): Destructible(maxHp, hp, defense, corpseName) {
    }

PlayerDestructible::PlayerDestructible(float maxHp, float hp, float defense, 
    const char *corpseName): Destructible(maxHp, hp, defense, corpseName) {
    }

void MonsterDestructible::die(Actor *owner) {
    engine.gui->message(TCODColor::darkRed, "%s is dead\n", owner->name);
    Destructible::die(owner);
}

void PlayerDestructible::die(Actor *owner) {
    engine.gui->message(TCODColor::darkRed, "You died!\n");
    Destructible::die(owner);
    engine.gameStatus = Engine::DEFEAT;
}