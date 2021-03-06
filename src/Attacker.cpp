#include <stdio.h>

#include "main.hpp"

Attacker::Attacker(float power): power(power) {
}

void Attacker::attack(Actor *owner, Actor *target) {
    if (target->destructible && !target->destructible->isDead()) {
        if (power - target->destructible->defense > 0) {
            engine.gui->message(TCODColor::yellow, "%s attacks %s for %g hp.\n", 
                owner->name, target->name, 
                power - target->destructible->defense);
        } else {
            engine.gui->message(TCODColor::yellow, "%s attacks %s but it has no effect!\n", owner->name, target->name);
        }
        target->destructible->takeDamage(target, power);
    } else {
        engine.gui->message(TCODColor::darkGrey, "%s attacks %s in vain.\n", owner->name, target->name);
    }
}
