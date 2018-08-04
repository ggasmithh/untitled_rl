class Pickable {
public:
    virtual ~Pickable() {};

    // the owner is the source, wearer the destination. change this to src and dest later
    bool pick(Actor *owner, Actor *wearer); 
    virtual bool use(Actor *owner, Actor *wearer);
};

class Healer: public Pickable {
public:
    float amount; // how much it heals for

    Healer(float amount);
    bool use(Actor *owner, Actor *wearer);
};