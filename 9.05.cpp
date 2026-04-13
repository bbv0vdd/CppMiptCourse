#include <cassert>
#include <memory>

template <typename T>
class List {
private:
    struct Node {
        explicit Node(const T& value) : data(value) {}
        
        T data;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;
    };

public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit Iterator(std::shared_ptr<Node> ptr = nullptr) : node(ptr) {}

        Iterator& operator++() {
            if (node) {
                node = node->next;
            }
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator& operator--() {
            if (node) {
                node = node->prev.lock();
            }
            return *this;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        reference operator*() const {
            return node->data;
        }

        pointer operator->() const {
            return &node->data;
        }

        friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
            return lhs.node == rhs.node;
        }

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
            return !(lhs == rhs);
        }

    private:
        std::shared_ptr<Node> node;
    };

    Iterator begin() const {
        return Iterator(head);
    }

    Iterator end() const {
        return Iterator();
    }

    void push_back(const T& value) {
        auto newNode = std::make_shared<Node>(value);
        
        if (!head) {
            head = newNode;
            return;
        }
        
        auto current = head;
        while (current->next) {
            current = current->next;
        }
        
        current->next = newNode;
        newNode->prev = current;
    }

private:
    std::shared_ptr<Node> head;
};

struct Entity {
    int value = 0;
};

int main() {
    List<int> numbers;
    
    numbers.push_back(1);
    numbers.push_back(2);
    numbers.push_back(3);
    
    auto it = numbers.begin();
    assert(*it == 1);
    ++it;
    assert(*it == 2);
    it++;
    assert(*it == 3);
    
    --it;
    assert(*it == 2);
    it--;
    assert(*it == 1);
    
    int sum = 0;
    for (int val : numbers) {
        sum += val;
    }
    assert(sum == 6);
    
    List<Entity> entities;
    entities.push_back(Entity{10});
    entities.push_back(Entity{20});
    
    auto entityIt = entities.begin();
    assert(entityIt->value == 10);
    ++entityIt;
    assert(entityIt->value == 20);
}