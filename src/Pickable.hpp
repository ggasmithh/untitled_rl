class Pickable: public Persistent {
public:
    virtual ~Pickable() {};

    // the owner is the source, wearer the destination. change this to src and dest later
    bool pick(Actor *owner, Actor *wearer);
    void drop(Actor *owner, Actor *wearer);
    virtual bool use(Actor *owner, Actor *wearer);

    static Pickable *create(TCODZip &zip);

protected:
    enum PickableType {
        HEALER, LIGHTNING_BOLT, CONFUSER, FIREBALL
    };
};

class Healer: public Pickable {
public:
    float amount; // how much it heals for

    Healer(float amount);
    bool use(Actor *owner, Actor *wearer);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class LightningBolt: public Pickable {
public:
    float range, damage;
    
    LightningBolt(float range, float damage);
    bool use(Actor *owner, Actor *wearer);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

/* i get why we are doing this from a code perspective 
but im not a big fan of it conceptually... */
class Fireball:public LightningBolt {
public:
    Fireball(float range, float damage);
    bool use(Actor *owner, Actor *wearer);

    void save(TCODZip &zip);
};

class Confuser: public Pickable {
public:
    int nbTurns;
    float range;
    Confuser(int nbTurns, float range);
    bool use(Actor *owner, Actor *wearer);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};
