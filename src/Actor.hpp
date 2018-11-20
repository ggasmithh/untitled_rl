class Actor: { 
public:
    int x,y; // position on map
    int ch; // ascii code
    const char *name; // actor's name (changable)
    TCODColor col; // color
    bool blocks; // does this actor block movement?
    
    // components
    struct Attacker *attacker; // can damage
    struct Destructible *destructible; // can be damaged
    struct Ai *ai; // self updates
    struct Pickable *pickable; // can be picked up
    struct Container *container; //  something that can contain actors

    Actor(int x, int y, int ch, const char *name, const TCODColor &col);
    ~Actor();
    
    float getDistance(int cx, int cy) const;
    void update();
    void render() const;

};