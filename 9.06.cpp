#include <iostream>
#include <boost/iterator/iterator_facade.hpp>

class Iterator : public boost::iterator_facade<
    Iterator,
    int,
    boost::forward_traversal_tag,
    int
>
{
private:
    int current;
    int next;
    int index;

public:
    Iterator() : current(0), next(1), index(0) {}
    Iterator(int n) : current(0), next(1), index(n) {}

private:
    friend class boost::iterator_core_access;
    
    int dereference() const {
        return current;
    }
    
    void increment() {
        int new_current = next;
        int new_next = current + next;
        current = new_current;
        next = new_next;
        index++;
    }
    
    bool equal(const Iterator& other) const {
        return index == other.index;
    }
};

int main() {
    Iterator begin;
    Iterator end(10);
    
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << '\n' << std::endl;
    
    return 0;
}