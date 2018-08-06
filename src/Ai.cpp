#include <stdio.h>
#include <math.h>

#include "main.hpp"

// max number of turns monster will follow player before giving up
static const int TRACKING_TURNS = 3;

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
    switch(ascii) {
        case 'g':  { // pick up item
            bool found = false;
            
            /* in the future, maybe rather than doing it this way, we could somehow
            store all the information about what is in a tile in a Tile object, so we wouldn't
            have to do so much searching through the actors list */
            for (Actor **it = engine.actors.begin(); 
                it != engine.actors.end(); it++) {

                Actor *actor = *it;

                if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
                    if (actor->pickable->pick(actor, owner)) {
                        found = true;
                        engine.gui->message(TCODColor::lightGrey, "You pick up the %s.",
                            actor->name);
                        break;
                    } else if (!found) {
                        found = true;
                        engine.gui->message(TCODColor::red, "Your inventory is full.");
                    }
                }
            }
            if (!found) {
                engine.gui->message(TCODColor::lightGrey, "There is nothing to pick up.");
            }
            engine.gameStatus = Engine::NEW_TURN;
            break;
        }

        case 'i': { // open the inventory
            Actor *actor = chooseFromInventory(owner);

            if (actor) {
                actor->pickable->use(actor, owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
            break;
        }

        case 'd': {  // drop item
            Actor *actor = chooseFromInventory(owner);

            if (actor) {
                actor->pickable->drop(actor, owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
            break;
        }
    }   
}

void PlayerAi::update(Actor *owner) {
    if (owner->destructible && owner->destructible->isDead()) {
        return;
    }

    int dx = 0, dy = 0;
    switch(engine.lastKey.vk) {
        case TCODK_UP: dy = -1; break;
        case TCODK_RIGHT: dx = 1; break;
        case TCODK_DOWN: dy = 1; break;
        case TCODK_LEFT: dx = -1; break;
        case TCODK_CHAR: handleActionKey(owner, engine.lastKey.c); break;
        default:break;
    }

    if (dx != 0 || dy != 0) {
        engine.gameStatus = Engine::NEW_TURN;
        if (moveOrAttack(owner, owner->x + dx, owner->y + dy)) {
            engine.map->computeFov();
        }
    }

}

bool PlayerAi::moveOrAttack(Actor *owner, int targetx, int targety) {
    if (engine.map->isWall(targetx, targety)) return false;

    // look for things to attack
    for (Actor **it=engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        if (actor->destructible && !actor->destructible->isDead() 
            && actor->x == targetx && actor->y == targety) {
            owner->attacker->attack(owner, actor);
            return false;
        }
    }

    // look for corpses and items
    for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        bool corpseOrItem = (actor->destructible && actor->destructible->isDead())
            || actor->pickable;
        if (corpseOrItem && actor->x == targetx && actor->y == targety) {
            engine.gui->message(TCODColor::darkGrey, "There's a %s here.\n", actor->name);
        }
    }

    owner->x = targetx;
    owner->y = targety;
    return true;
}

Actor *PlayerAi::chooseFromInventory(Actor *owner) {
    static const int INVENTORY_WIDTH = 50;
    static const int INVENTORY_HEIGHT = 28;
    static TCODConsole con(INVENTORY_WIDTH, INVENTORY_HEIGHT);

    // display the inventory frame
    con.setDefaultBackground(TCODColor(200, 180, 50));
    con.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, 
        TCOD_BKGND_DEFAULT, "inventory");
    
    // display items with keyboard shortcut
    con.setDefaultForeground(TCODColor::white);
    int shortcut = 'a';
    int y = 1;
    for (Actor **it=owner->container->inventory.begin();
        it != owner->container->inventory.end(); it++) {
        
        Actor *actor = *it;
        con.print(2, y, "(%c) %s", shortcut, actor->name);
        y++;
        shortcut++;
    }

    // blit the inventory console to the root console and flush
    TCODConsole::blit(&con, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, 
        TCODConsole::root, engine.screenWidth / 2 - INVENTORY_WIDTH / 2,
        engine.screenHeight / 2 - INVENTORY_HEIGHT / 2);
    TCODConsole::flush();

    // wait for a keypress (to select an item)
    TCOD_key_t key;
    TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);

    if (key.vk == TCODK_CHAR) {
        /* this is pretty cool. as I understand it, we're subracting the ascii
        value of the selected key from 'a', the first possible character that
        can be an item shortcut. if, for example, we picked "(a)", 97, and 
        subtracted the same amount, we would know that we picked the 0th item */
        int actorIndex = key.c - 'a';

        if (actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
            return owner->container->inventory.get(actorIndex);
        } 
    }
    return NULL;
}

void MonsterAi::update(Actor *owner) {
    if (owner->destructible && owner->destructible->isDead()) {
        return;
    }
    
    if (engine.map->isInFov(owner->x, owner->y)) {
        // we can see the player, start moving towards them
        moveCount = TRACKING_TURNS;
    } else {
        moveCount--;
    }

    if (moveCount > 0) {
        moveOrAttack(owner, engine.player->x, engine.player->y);
    }
}

void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety) {
    int dx = targetx - owner->x;
    int dy = targety - owner->y;

    // these next two will let us implement "wall sliding"
    int stepdx = (dx > 0 ? 1:-1);
    int stepdy = (dy > 0 ? 1:-1);
    float distance=sqrtf((dx * dx) + (dy * dy));

    if (distance >= 2) {
        dx = (int)(round(dx/distance));
        dy = (int)(round(dy/distance));

        if (engine.map->canWalk(owner->x + dx, owner->y + dy)) {
            owner->x += dx;
            owner->y += dy;
        } else if (engine.map->canWalk(owner->x + stepdx, owner->y)) {
            owner->x += stepdx;
        } else if (engine.map->canWalk(owner->x, owner->y + stepdy)) {
            owner->y += stepdy;
        }
    } else if (owner->attacker) {
        owner->attacker->attack(owner, engine.player);
    }
}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns, Ai *oldAi): 
    nbTurns(nbTurns), oldAi(oldAi) {
}

void ConfusedMonsterAi::update(Actor *owner) {
    TCODRandom *rng=TCODRandom::getInstance();
    int dx = rng->getInt(-1,1);
    int dy = rng->getInt(-1,1);

    int destx = owner->x + dx;
    int desty = owner->y + dy;
    if (engine.map->canWalk(destx, desty)) {
        owner->x = destx;
        owner->y = desty;
    } else {
        Actor *actor = engine.getActor(destx, desty);
        if (actor) {
            owner->attacker->attack(owner, actor);
        }
    }
    nbTurns--;
    if (nbTurns == 0) {
        owner->ai = oldAi;
        delete this;
    }
}

