/*
g++ -std=c++23 -Wall -Wextra 05.04.cpp -o 05.04.out
./05.04.out
*/

////////////////////////////////////////

#include <iostream>

////////////////////////////////////////

template <typename StrategyImpl>
class Entity : public StrategyImpl
{   
};

////////////////////////////////////////

class ClientStrategy
{
public:
    void test() const
    {
        std::cout << "ClientStrategy::test\n";
    }
    
    void execute() const
    {
        std::cout << "ClientStrategy::execute\n";
    }
};

////////////////////////////////////////

class ServerStrategy
{
public:
    void test() const
    {
        std::cout << "ServerStrategy::test\n";
    }
    
    void execute() const
    {
        std::cout << "ServerStrategy::execute\n";
    }
};

////////////////////////////////////////

template <typename EntityType>
void test_entity(EntityType const& entity)
{
    entity.test();
    entity.execute();
}

////////////////////////////////////////

int main()
{
    Entity<ClientStrategy> client_entity;
    std::cout << "Testing Client Entity:\n";
    test_entity(client_entity);
    
    Entity<ServerStrategy> server_entity;
    std::cout << "\nTesting Server Entity:\n";
    test_entity(server_entity);
    
    return 0;
}