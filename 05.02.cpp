/*
g++ -std=c++23 -Wall -Wextra 05.02.cpp -o 05.02.out
./05.02.out
*/

#include <iostream>

////////////////////////////////////////////////////

class Entity 
{
public :

    virtual ~Entity() = default;

//  ------------------------------

    virtual void test() const = 0;
};

////////////////////////////////////////////////////

class Client : public Entity 
{ 
public : 
    
    void test() const override 
    { 
        std::cout << "Client::test\n";
    } 
};

////////////////////////////////////////////////////

class Server : public Entity 
{ 
public : 
    
    void test() const override 
    { 
        std::cout << "Server::test\n";
    } 
};


////////////////////////////////////////////////////

template<typename T>
class Decorator : public T
{
public:

    void test() const override
    {
        std::cout << "Decorator::test : ";
        T::test();
    }
};

////////////////////////////////////////////////////

int main()
{
    Entity* entity_server = new Decorator<Server>;

    entity_server->test();

    delete entity_server;

    ////////////////////////////////////////////////////

    Entity* entity_client = new Decorator<Client>;

    entity_client->test();

    delete entity_client;

}

////////////////////////////////////////////////////