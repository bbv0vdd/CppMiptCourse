#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>


/*
 * TYPE is meaningless — type of what?
 * UPPERCASE enum names are typically for constants, not enum types
 * Should be enum class Side or enum class Direction
 */
 
/*
 * maybe:
	enum class Side
	{
		Left,
		Right
	};
*/

enum TYPE
{
    LEFT,
    RIGHT
};


class Tree
{
public:
    struct Node
    {
    public:
        explicit Node(const int value) : m_value(value)
        {
            std::cout << "Node(" << m_value << ") created\n";
        }

        ~Node()
        {
            std::cout << "Node(" << m_value << ") destroyed\n";
        }

        int m_value = 0;
        std::shared_ptr<Node> m_left{};
        std::shared_ptr<Node> m_right{};
        std::weak_ptr<Node> m_parent{};
    };

    std::shared_ptr<Node> m_root{};

    void traverse_v1() const
    {
        if (!m_root)
        {
            std::cout << "Breadth-First Search : Tree is empty\n";
            return;
        }

        std::queue<std::shared_ptr<Node>> nodes;
        nodes.push(m_root);

        std::cout << "Breadth-First Search : ";

        while (!nodes.empty())
        {
            const std::shared_ptr<Node> current = nodes.front();
            nodes.pop();

            std::cout << current->m_value << ' ';

            if (current->m_left)
                nodes.push(current->m_left);

            if (current->m_right)
                nodes.push(current->m_right);
        }

        std::cout << '\n';
    }

    void traverse_v2() const
    {
        if (!m_root)
        {
            std::cout << "Depth-First Search : Tree is empty\n";
            return;
        }

        std::stack<std::shared_ptr<Node>> nodes;
        nodes.push(m_root);

        std::cout << "Depth-First Search : ";

        while (!nodes.empty())
        {
            const std::shared_ptr<Node> current = nodes.top();
            nodes.pop();

            std::cout << current->m_value << ' ';

            if (current->m_right)
                nodes.push(current->m_right);

            if (current->m_left)
                nodes.push(current->m_left);
        }

        std::cout << '\n';
    }
};

static std::shared_ptr<Tree::Node> make_node(const int value)
{
    return std::make_shared<Tree::Node>(value);
}

static void connect(
    const std::shared_ptr<Tree::Node>& parent,
    const std::shared_ptr<Tree::Node>& child, TYPE type)
{
    assert(parent);
    assert(type == TYPE::LEFT || type == TYPE::RIGHT);
    if (type == TYPE::LEFT)
        parent->m_left = child;
    else
        parent->m_right = child;

    if (child)
        child->m_parent = parent;
}

static Tree make_demo_tree()
{
    Tree tree;

    tree.m_root = make_node(1);

    const std::shared_ptr<Tree::Node> left = make_node(2);
    const std::shared_ptr<Tree::Node> right = make_node(3);

    const std::shared_ptr<Tree::Node> left_left = make_node(4);
    const std::shared_ptr<Tree::Node> left_right = make_node(5);
    const std::shared_ptr<Tree::Node> right_left = make_node(6);
    const std::shared_ptr<Tree::Node> right_right = make_node(7);

    connect(tree.m_root, left, LEFT);
    connect(tree.m_root, right, RIGHT);

    connect(left, left_left, LEFT);
    connect(left, left_right, RIGHT);

    connect(right, right_left, LEFT);
    connect(right, right_right, RIGHT);

    return tree;
}

static void run_tests()
{
    Tree tree = make_demo_tree();

    assert(tree.m_root);
    assert(tree.m_root->m_value == 1); // why are you setting vlues again, are they not set already in make_demo_tree()?

    assert(tree.m_root->m_left);
    assert(tree.m_root->m_left->m_value == 2);

    assert(tree.m_root->m_right);
    assert(tree.m_root->m_right->m_value == 3);

    assert(tree.m_root->m_left->m_left);
    assert(tree.m_root->m_left->m_left->m_value == 4);

    assert(tree.m_root->m_left->m_right);
    assert(tree.m_root->m_left->m_right->m_value == 5);

    assert(tree.m_root->m_right->m_left);
    assert(tree.m_root->m_right->m_left->m_value == 6);

    assert(tree.m_root->m_right->m_right);
    assert(tree.m_root->m_right->m_right->m_value == 7);

    assert(!tree.m_root->m_parent.lock());
    assert(tree.m_root->m_left->m_parent.lock() == tree.m_root);
    assert(tree.m_root->m_right->m_parent.lock() == tree.m_root);

    tree.traverse_v1();
    tree.traverse_v2();
}

int main()
{
    run_tests();

    std::cout << "tests passed\n";
    std::cout << "destroying tree\n";

    Tree tree = make_demo_tree();

    tree.traverse_v1();
    tree.traverse_v2();

    std::cout << "root use_count : " << tree.m_root.use_count() << '\n';
    std::cout << "left parent alive : "
              << static_cast<bool>(tree.m_root->m_left->m_parent.lock())
              << '\n';

    std::cout << "reset root\n";
    tree.m_root.reset();

    std::cout << "done\n";
}


// score is 7/10
