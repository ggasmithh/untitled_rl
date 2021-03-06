#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;

class BspListener : public ITCODBspCallback {
private:
    Map &map;
    int roomNum;
    int lastx, lasty; // CENTER of the last room

public:
    BspListener(Map &map) : map(map), roomNum(0) {}
    bool visitNode(TCODBsp *node, void *userData) {
        if (node->isLeaf()) {
            int x, y, w, h;
            bool withActors = (bool)userData;

            // room digging
            w = map.rng->getInt(ROOM_MIN_SIZE, node->w - 2);
            h = map.rng->getInt(ROOM_MIN_SIZE, node->h - 2);
            x = map.rng->getInt(node-> x + 1, node-> x + node->w - w - 1);
            y = map.rng->getInt(node->y + 1, node->y+node->h - h - 1);
            map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1, withActors);
            // if this isnt the first room, connect it to the last room
            if (roomNum != 0) {
                // first arm
                map.dig(lastx, lasty, x + w / 2, lasty);
                
                // second arm
                map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
            }

        lastx = x + w / 2;
        lasty = y + h / 2;
        roomNum++;

        }
        return true;
    }

};

 Map::Map(int width, int height): width(width),height(height) {
    // 0x7FFFFFFF is the highest 32 bit signed int
    seed = TCODRandom::getInstance()->getInt(0, 0x7FFFFFFF);
 }

Map::~Map() {
    delete [] tiles;
    delete map;
}

void Map::init(bool withActors) {
    rng = new TCODRandom(seed, TCOD_RNG_CMWC);
    tiles = new Tile[width * height];
    map = new TCODMap(width, height);

    TCODBsp bsp(0, 0, width, height);
    bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
    BspListener listener(*this);
    /*  reading about casting as (void *) confuses me but it works and it's 
        what this library wants, so.... */
    bsp.traverseInvertedLevelOrder(&listener, (void *)withActors);
}

bool Map::isWall(int x, int y) const {
    return !map->isWalkable(x, y);
}

bool Map::isExplored(int x, int y) const {
    return tiles[x + y * width].explored;
}

bool Map::isInFov(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }

    if (map->isInFov(x, y)) {
        tiles[x + y * width].explored = true;
        return true;
    }
    
    return false;
}

void Map::computeFov() {
    map->computeFov(engine.player->x, engine.player->y, engine.fovRadius);
}

bool Map::canWalk(int x, int y) const {
    if (isWall(x, y)) {
        return false;
    }

    for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
        Actor *actor = *it;
        if (actor->blocks && actor->x == x && actor->y == y) {
            return false;
        }
    }
    return true;
}

void Map::addMonster(int x, int y) {
    TCODRandom *rng = TCODRandom::getInstance();
    if (rng->getInt(0, 100) < 80) {
        // create an orc
        Actor *orc = new Actor(x, y, 'o', "orc", TCODColor::desaturatedGreen);
        orc->destructible = new MonsterDestructible(10, 10, 0, "dead orc");
        orc->attacker = new Attacker(3);
        orc->ai = new MonsterAi();
        engine.actors.push(orc);
    } else {
        Actor *troll = new Actor(x, y, 'T', "troll", TCODColor::darkerGreen);
        troll->destructible = new MonsterDestructible(16, 16, 1, "dead troll");
        troll->attacker = new Attacker(4);
        troll->ai = new MonsterAi();
        engine.actors.push(troll);
    }
}

void Map::addItem(int x, int y) {
    TCODRandom *rng = TCODRandom::getInstance();
    int dice = rng->getInt(0, 100);
    
    // 70% chance for a health potion
    if (dice < 70) {
        Actor *healthPotion = new Actor(x, y, '!', 
            "health potion", TCODColor::violet);
        healthPotion->blocks = false;
        healthPotion->pickable = new Healer(4);
        engine.actors.insertBefore(healthPotion, 0);
        
    // 10% chance for a scroll of lightning bolt
    } else if (dice < 80) {
        Actor *scrollOfLightningBolt = new Actor(x, y, '#', 
            "scroll of lightning bolt", TCODColor::lightYellow);
        scrollOfLightningBolt->blocks = false;
        scrollOfLightningBolt->pickable = new LightningBolt(5, 20);
        engine.actors.insertBefore(scrollOfLightningBolt, 0);

    // 10% chance for a scroll of fireball
    } else if (dice < 90) {
        Actor *scrollOfFireball = new Actor(x, y, '#', 
            "scroll of fireball", TCODColor::lightYellow);
        scrollOfFireball->blocks = false;
        scrollOfFireball->pickable = new Fireball(3, 12);
        engine.actors.insertBefore(scrollOfFireball, 0);
    
    // 10% chance for scroll of confusion
    } else {
        Actor *scrollOfConfusion = new Actor(x, y, '#',
            "scroll of confusion", TCODColor::lightYellow);
        scrollOfConfusion->blocks = false;
        scrollOfConfusion->pickable = new Confuser(10, 8);
        engine.actors.insertBefore(scrollOfConfusion, 0);
    }
}

void Map::render() const {
    static const TCODColor darkWall(0,0,100);
    static const TCODColor darkGround(50,50,150);
    static const TCODColor lightWall(130,110,50);
    static const TCODColor lightGround(200,180,50);
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {    
            if (isInFov(x, y)) {
                TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
            } else if (isExplored(x, y)) {
                TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
            }
           
        }
    }
}

void Map::dig(int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }

    if (y2 < y1) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }

    for (int tilex = x1; tilex <= x2; tilex++) {
        for (int tiley = y1; tiley <= y2; tiley++) {
            map->setProperties(tilex, tiley, true, true);
        }
    }
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors) {
    dig(x1, y1, x2, y2);

    if (!withActors) {return;}
    
    // put the player in the middle of the first room
    if (first) {
        engine.player->x = (x1 + x2) / 2;
        engine.player->y = (y1 + y2) / 2;

    // if this isnt the first room, add some monsters and health potions
    } else {
        TCODRandom *rng = TCODRandom::getInstance();
        int nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
        while (nbMonsters > 0) {
            int x = rng->getInt(x1, x2);
            int y = rng->getInt(y1, y2);
            if (canWalk(x, y)) {
                addMonster(x, y);
            }
            nbMonsters--;
        }

        int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
        while (nbItems > 0) {
            int x = rng->getInt(x1, x2);
            int y = rng->getInt(y1, y2);
            if (canWalk(x,y)) {
                addItem(x, y);
            }
            nbItems--;
        }
    }
}
