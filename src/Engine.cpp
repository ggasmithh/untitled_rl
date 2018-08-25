#include "libtcod.hpp"

#include "main.hpp"

Engine::Engine(int screenWidth, int screenHeight): 
    gameStatus(STARTUP), fovRadius(10), 
    screenWidth(screenWidth), screenHeight(screenHeight) {

    TCODConsole::initRoot(80,50,"libtcod C++ tutorial",false);
    gui = new Gui();
}

Engine::~Engine() {
    actors.clearAndDelete();
    delete map;
    delete gui;
}

void Engine::init() {
    player = new Actor(40, 25, '@', "player", TCODColor::white);
    player->destructible = new PlayerDestructible(30, 30, 2, "your cadaver");
    player->attacker = new Attacker(5);
    player->ai = new PlayerAi();
    player->container = new Container(26); // one slot for each letter
    actors.push(player);
    map = new Map(80, 45);
    map->init(true);
}

void Engine::load() {
    if (TCODSystem::fileExists("game.sav")) {
        TCODZip zip;
        zip.loadFromFile("game.sav");
        
        // load map
        int width = zip.getInt();
        int height = zip.getInt();
        map->load(zip);

        // load player
        player = new Actor(0, 0, 0, NULL, TCODColor::white);
        player->load(zip);
        actors.push(player);

        // load the other actors
        int nbActors = zip.getInt();
        while (nbActors > 0) {
            Actor *actor = new Actor(0, 0, 0, NULL, TCODColor::white);
            actor->load(zip);
            actors.push(actor);
            nbActors--;
        }

        // load the message log
        gui->load(zip);
    } else {
        engine.init();
    }

}

void Engine::save() {
    if (player->destructible->isDead()) {
        TCODSystem::deleteFile("game.sav");
    } else {
        TCODZip zip;
        // save map
        zip.putInt(map->width);
        zip.putInt(map->height);
        map->save(zip);

        // save player
        player->save(zip);

        // save other actors
        zip.putInt(actors.size() - 1);
        for (Actor **it = actors.begin(); it != actors.end(); it++) {
            if (*it != player) {
                (*it)->save(zip);
            }
        }

        // save message log
        gui->save(zip);

        // compressed data to file
        zip.saveToFile("game.sav");
    }
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
    // 0 range is infinite
    Actor *closest = NULL;
    float bestDistance = 1E6f; // we are starting out absurdly high

    for (Actor **it = actors.begin(); it != actors.end(); it++) {
        Actor *actor = *it;

        if (actor != player && actor->destructible 
            && !actor->destructible->isDead()) {
            
            float distance = actor->getDistance(x, y);

            if (distance < bestDistance && (distance <= range || range == 0.0f)) {
                bestDistance = distance;
                closest = actor;
            }
        }
    }
    return closest;
}

Actor *Engine::getActor(int x, int y) const {
    for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        if (actor->x == x && actor->y == y && actor->destructible 
            && !actor->destructible->isDead()) {
                return actor;
            }
    }
    return NULL;
}

bool Engine::pickATile(int *x, int *y, float maxRange) {
    // 0 range = player FOV

    // we need to render while the player is picking
    while (!TCODConsole::root->isWindowClosed()) {
        render();

        // highlight the possible range
        for (int cx = 0; cx < map->width; cx++) {
            for (int cy = 0; cy < map->height; cy++) {
                if (map->isInFov(cx, cy) && (maxRange == 0 
                    || player->getDistance(cx, cy) <= maxRange)) {
                    
                    TCODColor col = TCODConsole::root->getCharBackground(cx, cy);
                    col = col * 1.2f;
                    TCODConsole::root->setCharBackground(cx, cy, col);
                }
            }
        }
        TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,
            &lastKey,&mouse);

        if ((map->isInFov(mouse.cx, mouse.cy) && maxRange == 0) 
            || player->getDistance(mouse.cx, mouse.cy) <= maxRange) {
            
            TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, 
                TCODColor::white);

            if (mouse.lbutton_pressed) {
                *x = mouse.cx;
                *y = mouse.cy;
                return true;
            }
        }

        if (mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) {
            return false;
        }
        TCODConsole::flush();
    }
    return false;
}

void Engine::sendToBack(Actor *actor) {
    actors.remove(actor);
    actors.insertBefore(actor, 0);
}


void Engine::update() {
    if (gameStatus == STARTUP) map->computeFov();
    gameStatus = IDLE;

    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse); 
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