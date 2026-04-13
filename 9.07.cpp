#include <cassert>
#include <iostream>
#include <memory>
#include <tuple>

class Entity_v1
{
public:
    auto make_shared()
    { 
        return std::shared_ptr<Entity_v1>(this);
    }
};

class Entity_v2 : public std::enable_shared_from_this<Entity_v2>
{
public:
    auto make_shared()
    { 
        return shared_from_this();
    }
};

class Entity_v3 : public std::enable_shared_from_this<Entity_v3>
{
private:
    struct Key {};
    
public:
    Entity_v3(Key) {}
    
    auto make_shared()
    { 
        return shared_from_this();
    }
    
    static std::shared_ptr<Entity_v3> create()
    {
        return std::make_shared<Entity_v3>(Key());
    }
};

int main()
{
    auto v1_1 = std::make_shared<Entity_v1>();
    // auto v1_2 = v1_1->make_shared(); // double deletion!
    
    auto v2_1 = std::make_shared<Entity_v2>();
    auto v2_2 = v2_1->make_shared();
    
    assert(v2_1.use_count() == 2);
    assert(v2_2.use_count() == 2);
    
    try
    {
        Entity_v2 v2_on_stack;
        v2_on_stack.make_shared();
    }
    catch (std::bad_weak_ptr const& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    
    auto v3_1 = Entity_v3::create();
    auto v3_2 = v3_1->make_shared();
    
    assert(v3_1.use_count() == 2);
    assert(v3_2.use_count() == 2);
}