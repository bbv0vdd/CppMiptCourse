/*
g++ -std=c++23 -Wall -Wextra 05.01.cpp -o 05.01.out
./05.01.out
*/

////////////////////////////////////////////////////

#include <string>
#include <cassert>

////////////////////////////////////////////////////


class Person
{
public:
    Person() = default;
    
    Person(std::string name, int age, int grade)
        : m_name(std::move(name))
        , m_age(age)
        , m_grade(grade)
    {
    }
    
    const std::string& get_name() const
    {
        return m_name;
    }
    
    int get_age() const
    {
        return m_age;
    }
    
    int get_grade() const
    {
        return m_grade;
    }

private:
    std::string m_name;
    int m_age = 0;
    int m_grade = 0;
};

class Builder
{
public:
    Builder() = default;
    
    Builder& name(const std::string& name_value)
    {
        m_name = name_value;
        return *this;
    }
    
    Builder& age(int age_value)
    {
        m_age = age_value;
        return *this;
    }
    
    Builder& grade(int grade_value)
    {
        m_grade = grade_value;
        return *this;
    }
    
    Person get()
    {
        return Person(m_name, m_age, m_grade);
    }

private:
    std::string m_name;
    int m_age = 0;
    int m_grade = 0;
};

int main()
{
    {
       Builder builder;
        Person person = builder.name("Ivan").age(25).grade(10).get();
        
        assert(person.get_name() == "Ivan");
        assert(person.get_age() == 25);
        assert(person.get_grade() == 10);
    }

    {
        Person person = Builder().get();
        
        assert(person.get_name() == "");
        assert(person.get_age() == 0);
        assert(person.get_grade() == 0);
    }

    return 0;
}