#include "main.hpp"

bool Pickable::pick(Actor *owner, Actor *wearer) {

    // if the destination has a container, and we can add the actor, return true
    if (wearer->container && wearer->container->add(owner)) {
        engine.actors.remove(owner);
        return true;
    }
    return false;
}

bool Pickable::use(Actor *owner, Actor *wearer) {
    if (wearer->container) {
        wearer->container->remove(owner);
        delete owner;
        return true;
    }
    return false;
}

void Pickable::drop(Actor *owner, Actor *wearer) {
    if (wearer->container) {
        wearer->container->remove(owner);
        engine.actors.insertBefore(owner, 0);
        owner->x = wearer->x;
        owner->y = wearer->y;
        engine.gui->message(TCODColor::lightGrey, "%s drops a %s", wearer->name, owner->name);
    }
}

Healer::Healer(float amount): amount(amount) {
}

bool Healer::use(Actor *owner, Actor *wearer) {
    if (wearer->destructible) {
        float amountHealed = (wearer->destructible->heal(amount));
        if (amountHealed > 0) {
            return Pickable::use(owner, wearer);
        }
    }
    
    return false;
}

LightningBolt::LightningBolt(float range, float damage): 
    range(range), damage(damage) {
}

bool LightningBolt::use(Actor *owner, Actor *wearer) {
    Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
    if (!closestMonster) {
        engine.gui->message(TCODColor::lightGrey, "No enemy in range.");
        return false;
    }

    return Pickable::use(owner, wearer);
}

/* again, tying Fireball to Lightning bolt doesnt 
feel comfy. Maybe i should make a Spell class? */
Fireball::Fireball(float range, float damage):
    LightningBolt(range, damage) {
}

bool Fireball::use(Actor *owner, Actor *wearer) {
    engine.gui->message(TCODColor::cyan, "Left-click a target for Fireball.");
    int x, y;
    if (!engine.pickATile(&x,&y)) {
        return false;
    }
    // burn everything in range of the fireball (everything)
    engine.gui->message(TCODColor::orange, 
        "The fireball explodes, burning everything within %g tiles!", range);
    
    for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        if (actor->destructible && !actor->destructible->isDead()
            && actor->getDistance(x, y) <= range) {

        engine.gui->message(TCODColor::orange, "The %s gets burned for %g hit points.",
            actor->name, damage);
        actor->destructible->takeDamage(actor, damage);
        }
    }
    return Pickable::use(owner, wearer);
}

Confuser::Confuser(int nbTurns, float range): nbTurns(nbTurns), range(range) {
}

bool Confuser::use(Actor *owner, Actor *wearer) {
    engine.gui->message(TCODColor::cyan, "Left-click an enemy to confuse it.");
    int x, y;
    if (!engine.pickATile(&x, &y, range)) {
        return false;
    }
    Actor *actor = engine.getActor(x, y);
    if (!actor) {
        return false;
    }
    Ai *confusedAi = new ConfusedMonsterAi(nbTurns, actor->ai);
    actor->ai = confusedAi;
    engine.gui->message(TCODColor::lightGreen, "The %s begins to stumble around!", 
        actor->name);
    return Pickable::use(owner, wearer);
}

Pickable *Pickable::create(TCODZip &zip) {
    PickableType type = (PickableType)zip.getInt();
    Pickable *pickable = NULL;
    switch (type) {
        case HEALER: pickable = new Healer(0); break;
        case LIGHTNING_BOLT: pickable = new LightningBolt(0, 0); break;
        case CONFUSER: pickable = new Confuser(0, 0); break;
        case FIREBALL: pickable = new Fireball(0, 0); break;
    }
    
    return pickable;
}

