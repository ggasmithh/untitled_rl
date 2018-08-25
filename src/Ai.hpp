class Ai: public Persistent {
public:
    // the ... = 0 makes this an abstract/purely virtual class
    virtual void update(Actor *owner) = 0;

    static Ai *create(TCODZip &zip);

protected:
    enum AiType {
        PLAYER, MONSTER, CONFUSED_MONSTER
    };
};

class PlayerAi: public Ai {
public:

    void handleActionKey(Actor *owner, int ascii);
    void update(Actor *owner);

    void load(TCODZip &zip);
    void save(TCODZip &zip);

protected:
    Actor *chooseFromInventory(Actor *owner);
    bool moveOrAttack(Actor *owner, int targetx, int targety);
};

class MonsterAi: public Ai {
public:
    void update(Actor *owner);

    void load(TCODZip &zip);
    void save(TCODZip &zip);

protected:
    int moveCount;
    void moveOrAttack(Actor *owner, int targetx, int targety);
};

class ConfusedMonsterAi: public Ai {
public:
    ConfusedMonsterAi(int nbTurns, Ai *oldAi);
    void update(Actor *owner);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
    
protected:
    int nbTurns;
    Ai *oldAi;
};