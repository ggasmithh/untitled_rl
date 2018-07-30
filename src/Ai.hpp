class Ai {
public:
    // the ... = 0 makes this an abstract/purely virtual class
    virtual void update(Actor *owner) = 0;
};

class PlayerAi: public Ai {
public:
    void update(Actor *owner);

protected:
    bool moveOrAttack(Actor *owner, int targetx, int targety);
};

class MonsterAi: public Ai {
public:
    void update(Actor *owner);

protected:
    void moveOrAttack(Actor *owner, int targetx, int targety);
};