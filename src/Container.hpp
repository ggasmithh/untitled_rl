class Container {
public:
    int size;   // storage size. 0 = infinite
    TCODList<Actor *> inventory;

    Container(int size);
    ~Container();

    bool add(Actor *actor);
    void remove(Actor *actor);

};