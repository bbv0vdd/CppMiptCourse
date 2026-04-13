/*
g++ -std=c++23 -Wall -Wextra -Wpedantic 09.03.cpp -o 09.03.out
./09.03.out
*/

#include <cstddef>
#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

struct Entity_Simple
{
    int x = 0;
    int y = 0;
};

class EntityBuilder
{
public:
    virtual ~EntityBuilder() = default;
    
    std::unique_ptr<Entity_Simple> build()
    {
        m_entity = std::make_unique<Entity_Simple>();
        set_x();
        set_y();
        return std::move(m_entity);
    }
    
    virtual void set_x() const = 0;
    virtual void set_y() const = 0;

protected:
    mutable std::unique_ptr<Entity_Simple> m_entity{};
};

class ClientBuilder : public EntityBuilder
{
public:
    void set_x() const override { m_entity->x = 1; }
    void set_y() const override { m_entity->y = 1; }
};

class ServerBuilder : public EntityBuilder
{
public:
    void set_x() const override { m_entity->x = 2; }
    void set_y() const override { m_entity->y = 2; }
};

void test_builder()
{
    std::unique_ptr<EntityBuilder> builder = std::make_unique<ClientBuilder>();
    std::unique_ptr<Entity_Simple> entity = builder->build();
    
    assert(entity && entity->x == 1 && entity->y == 1);
    
    builder = std::make_unique<ServerBuilder>();
    entity = builder->build();
    
    assert(entity && entity->x == 2 && entity->y == 2);
    std::cout << "\tBuilder Pattern: OK\n";
}

class Entity_Base
{
public:
    virtual ~Entity_Base() = default;
    virtual int get_type() const = 0;
};

class EntityClient : public Entity_Base
{
public:
    int get_type() const override { return 1; }
};

class EntityServer : public Entity_Base
{
public:
    int get_type() const override { return 2; }
};

class EntityFactory
{
public:
    virtual ~EntityFactory() = default;
    virtual std::unique_ptr<Entity_Base> create() const = 0;
};

class ClientFactory : public EntityFactory
{
public:
    std::unique_ptr<Entity_Base> create() const override
    {
        return std::make_unique<EntityClient>();
    }
};

class ServerFactory : public EntityFactory
{
public:
    std::unique_ptr<Entity_Base> create() const override
    {
        return std::make_unique<EntityServer>();
    }
};

void test_factory()
{
    std::unique_ptr<EntityFactory> factory = std::make_unique<ClientFactory>();
    std::unique_ptr<Entity_Base> entity = factory->create();
    
    assert(entity && entity->get_type() == 1);
    
    factory = std::make_unique<ServerFactory>();
    entity = factory->create();
    
    assert(entity && entity->get_type() == 2);
    std::cout << "\tFactory Pattern: OK\n";
}

class CloneableEntity
{
public:
    virtual ~CloneableEntity() = default;
    virtual std::unique_ptr<CloneableEntity> clone() const = 0;
    virtual int get_code() const = 0;
};

class PrototypeClient : public CloneableEntity
{
public:
    std::unique_ptr<CloneableEntity> clone() const override
    {
        return std::make_unique<PrototypeClient>(*this);
    }
    
    int get_code() const override { return 1; }
};

class PrototypeServer : public CloneableEntity
{
public:
    std::unique_ptr<CloneableEntity> clone() const override
    {
        return std::make_unique<PrototypeServer>(*this);
    }
    
    int get_code() const override { return 2; }
};

class PrototypeRegistry
{
public:
    PrototypeRegistry()
    {
        m_prototypes.push_back(std::make_unique<PrototypeClient>());
        m_prototypes.push_back(std::make_unique<PrototypeServer>());
    }
    
    std::unique_ptr<CloneableEntity> create_client() const
    {
        return m_prototypes[0]->clone();
    }
    
    std::unique_ptr<CloneableEntity> create_server() const
    {
        return m_prototypes[1]->clone();
    }

private:
    std::vector<std::unique_ptr<CloneableEntity>> m_prototypes{};
};

void test_prototype()
{
    PrototypeRegistry registry;
    
    std::unique_ptr<CloneableEntity> client = registry.create_client();
    std::unique_ptr<CloneableEntity> server = registry.create_server();
    
    assert(client && server);
    assert(client->get_code() == 1);
    assert(server->get_code() == 2);
    assert(client.get() != server.get());
    
    std::cout << "\tPrototype Pattern: OK\n";
}

class Component
{
public:
    virtual ~Component() = default;
    virtual int compute() const = 0;
};

class LeafClient : public Component
{
public:
    int compute() const override { return 1; }
};

class LeafServer : public Component
{
public:
    int compute() const override { return 2; }
};

class CompositeNode : public Component
{
public:
    void attach(std::unique_ptr<Component> component)
    {
        m_children.push_back(std::move(component));
    }
    
    int compute() const override
    {
        int total = 0;
        for (const auto& child : m_children)
        {
            if (child) total += child->compute();
        }
        return total;
    }

private:
    std::vector<std::unique_ptr<Component>> m_children{};
};

std::unique_ptr<Component> create_composite_group(size_t clients, size_t servers)
{
    auto group = std::make_unique<CompositeNode>();
    
    for (size_t i = 0; i < clients; ++i)
        group->attach(std::make_unique<LeafClient>());
    
    for (size_t i = 0; i < servers; ++i)
        group->attach(std::make_unique<LeafServer>());
    
    return group;
}

void test_composite()
{
    auto root = std::make_unique<CompositeNode>();
    
    constexpr size_t GROUP_COUNT = 5;
    constexpr size_t CLIENTS_PER_GROUP = 1;
    constexpr size_t SERVERS_PER_GROUP = 1;
    constexpr int EXPECTED = 15;
    
    for (size_t i = 0; i < GROUP_COUNT; ++i)
    {
        root->attach(create_composite_group(CLIENTS_PER_GROUP, SERVERS_PER_GROUP));
    }
    
    std::unique_ptr<Component> component = std::move(root);
    assert(component && component->compute() == EXPECTED);
    
    std::cout << "\tComposite Pattern: OK\n";
}

class EventListener
{
public:
    virtual ~EventListener() = default;
    virtual void on_update(int value) const = 0;
};

class Observable
{
public:
    void subscribe(std::shared_ptr<EventListener> listener)
    {
        m_listeners.push_back(listener);
    }
    
    void notify(int value)
    {
        m_data = value;
        broadcast();
    }
    
    size_t get_listener_count() const { return m_listeners.size(); }

private:
    void broadcast() const
    {
        for (const auto& listener : m_listeners)
        {
            if (listener) listener->on_update(m_data);
        }
    }
    
    int m_data = 0;
    std::vector<std::shared_ptr<EventListener>> m_listeners{};
};

class ConsoleClient : public EventListener
{
public:
    void on_update(int value) const override
    {
        std::cout << "\tConsoleClient received: " << value << '\n';
    }
};

class ConsoleServer : public EventListener
{
public:
    void on_update(int value) const override
    {
        std::cout << "\tConsoleServer received: " << value << '\n';
    }
};

void test_observer()
{
    Observable observable;
    
    auto client = std::make_shared<ConsoleClient>();
    auto server = std::make_shared<ConsoleServer>();
    
    assert(client.use_count() == 1);
    assert(server.use_count() == 1);
    
    observable.subscribe(client);
    observable.subscribe(server);
    
    assert(observable.get_listener_count() == 2);
    assert(client.use_count() == 2);
    assert(server.use_count() == 2);
    
    for (int i = 1; i <= 2; ++i)
        observable.notify(i);
    
    std::cout << "\tObserver Pattern: OK\n";
}

int main()
{
    std::cout << "Design Patterns:\n";
    
    test_builder();
    test_factory();
    test_prototype();
    test_composite();
    test_observer();
    
    std::cout << "All tests passed\n";
}