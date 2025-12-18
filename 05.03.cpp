/*
g++ -std=c++23 -Wall -Wextra 05.03.cpp -o 05.03.out
./05.03.out
*/

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Composite
class Unit {
public:
    virtual ~Unit() = default;
    virtual int strength() const = 0;
    virtual void show() const = 0;
};

class Soldier : public Unit {
public:
    Soldier(int s) : power(s) {}
    int strength() const override { 
        return power; 
    }
    void show() const override { 
        std::cout << "Soldier(" << power << ")"; 
    }
private:
    int power;
};

class Squad : public Unit {
public:
    void add(std::unique_ptr<Unit> u) { 
        units.push_back(std::move(u)); 
    }
    int strength() const override {
        int total = 0;
        for (const auto& u : units) 
            total += u->strength();
        return total;
    }
    void show() const override {
        std::cout << "Squad[" << units.size() << "]: ";
        for (const auto& u : units) { 
            u->show(); 
            std::cout << " "; 
        }
    }
private:
    std::vector<std::unique_ptr<Unit>> units;
};

// Builder
class UnitBuilder {
public:
    virtual ~UnitBuilder() = default;
    virtual std::unique_ptr<Unit> create() = 0;
};

class InfantryBuilder : public UnitBuilder {
public:
    std::unique_ptr<Unit> create() override {
        return std::make_unique<Soldier>(10);
    }
};

class ArcherBuilder : public UnitBuilder {
public:
    std::unique_ptr<Unit> create() override {
        return std::make_unique<Soldier>(7);
    }
};

// Template Method
class Battle {
public:
    virtual ~Battle() = default;
    void execute() {
        prepare();
        action();
        finish();
    }
private:
    virtual void action() = 0;
    void prepare() { std::cout << "Prepare... "; }
    void finish() { std::cout << " Finish!\n"; }
};

class Attack : public Battle {
    void action() override { std::cout << "Attack!"; }
};

class Retreat : public Battle {
    void action() override { std::cout << "Retreat!"; }
};

// Example
int main() {
    std::cout << "=== Composite ===\n";
    auto squad = std::make_unique<Squad>();
    squad->add(std::make_unique<Soldier>(5));
    squad->add(std::make_unique<Soldier>(8));
    squad->show();
    std::cout << "\nTotal: " << squad->strength() << "\n\n";

    std::cout << "=== Builder ===\n";
    InfantryBuilder infantry;
    ArcherBuilder archer;
    auto unit1 = infantry.create();
    auto unit2 = archer.create();
    unit1->show(); std::cout << "\n";
    unit2->show(); std::cout << "\n\n";

    std::cout << "=== Template Method ===\n";
    Attack attack;
    Retreat retreat;
    attack.execute();
    retreat.execute();

    std::cout << "\n=== Combined ===\n";
    auto army = std::make_unique<Squad>();
    army->add(infantry.create());
    army->add(archer.create());
    army->show();
    std::cout << "\nArmy strength: " << army->strength() << "\n";

    return 0;
}