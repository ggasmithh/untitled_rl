class Actor { 
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


    Actor(int x, int y, int ch, const char *name, const TCODColor &col);
    void update();
    void render() const;
};