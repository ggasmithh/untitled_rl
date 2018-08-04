#include "libtcod.hpp"

#include "main.hpp"

Engine::Engine(int screenWidth, int screenHeight): gameStatus(STARTUP), 
    fovRadius(10), screenWidth(screenWidth), screenHeight(screenHeight) {
    TCODConsole::initRoot(80,50,"libtcod C++ tutorial",false);
    player = new Actor(40, 25, '@', "player", TCODColor::white);
    player->destructible = new PlayerDestructible(30, 30, 2, "your cadaver");
    player->attacker = new Attacker(5);
    player->ai = new PlayerAi();
    player->container = new Container(26); // one slot for each letter
    actors.push(player);
    map = new Map(80, 45);
    gui = new Gui();
}

Engine::~Engine() {
    actors.clearAndDelete();
    delete map;
    delete gui;
}

void Engine::sendToBack(Actor *actor) {
    actors.remove(actor);
    actors.insertBefore(actor, 0);
}


void Engine::update() {
    if (gameStatus == STARTUP) map->computeFov();
    gameStatus = IDLE;

    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&lastKey,NULL); 
    player->update();
    
    if (gameStatus == NEW_TURN) {
        for (Actor **it = actors.begin(); it != actors.end(); it++) {
            Actor *actor = *it;
            if (actor != player) {
                actor->update();
            }
        }
    }
}

void Engine::render() {
    TCODConsole::root->clear();
    
    map->render();

    for (Actor **it=actors.begin(); it != actors.end(); it++) {
        Actor *actor = *it;
        if (map->isInFov(actor->x, actor->y)) {
            actor->render();
        }
    }
    
    player->render();
    // player stats
    gui->render();
}