#include <stdio.h>

#include "main.hpp"

Actor::Actor(int x, int y, int ch, const char *name, const TCODColor &col) :
   x(x), y(y), ch(ch), name(name), col(col), 
   blocks(true), attacker(NULL), destructible(NULL), ai(NULL) {

}

void Actor::update() {
    if (ai) ai->update(this);
}

/* bool Actor::moveOrAttack(int x, int y) {
    if (engine.map->isWall(x, y)) return false;
    
    for (Actor **it=engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        
        if (actor->x == x && actor->y == y) {
            printf("The %s laughs at your efforts to attack him!\n", actor->name);
            return false;
        }
    }
    this->x = x;
    this->y = y;
    return true;
} */

void Actor::render() const {
   TCODConsole::root->setChar(x,y,ch);
   TCODConsole::root->setCharForeground(x,y,col);
}