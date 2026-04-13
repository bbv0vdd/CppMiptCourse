/*
g++ -std=c++23 -Wall -Wextra -Wpedantic 9.09.cpp $(pkg-config --cflags --libs benchmark) -o 9.09.out
./9.09.out
*/

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <utility>
#include <vector>
#include <chrono>

#include <benchmark/benchmark.h>

template<typename Policy>
class Allocator;

class FirstFitPolicy {
public:
    template<typename AllocatorType>
    static auto find(AllocatorType* allocator, std::size_t size) 
        -> std::pair<typename AllocatorType::Node*, typename AllocatorType::Node*> {
        
        auto* current = allocator->get_head();
        typename AllocatorType::Node* previous = nullptr;

        while (current != nullptr && current->size < size) {
            previous = current;
            current = current->next;
        }

        return {current, previous};
    }
};

class BestFitPolicy {
public:
    template<typename AllocatorType>
    static auto find(AllocatorType* allocator, std::size_t size) 
        -> std::pair<typename AllocatorType::Node*, typename AllocatorType::Node*> {
        
        typename AllocatorType::Node* best = nullptr;
        typename AllocatorType::Node* best_previous = nullptr;
        auto* current = allocator->get_head();
        typename AllocatorType::Node* previous = nullptr;

        while (current != nullptr) {
            if (current->size >= size) {
                if (best == nullptr || current->size < best->size) {
                    best = current;
                    best_previous = previous;
                }
            }
            previous = current;
            current = current->next;
        }

        return {best, best_previous};
    }
};

template<typename SearchPolicy = FirstFitPolicy>
class Allocator {
public:
    struct Node {
        std::size_t size = 0U;
        Node* next = nullptr;
    };

    struct alignas(std::max_align_t) Header {
        std::size_t size = 0U;
    };

    explicit Allocator(std::size_t size) : m_size(size) {
        assert(m_size >= sizeof(Node) + sizeof(Header) + 1U);
        m_begin = ::operator new(m_size, std::align_val_t(s_alignment));
        m_head = get_node(m_begin);
        m_head->size = m_size - sizeof(Header);
        m_head->next = nullptr;
    }

    ~Allocator() {
        ::operator delete(m_begin, std::align_val_t(s_alignment));
    }

    Allocator(const Allocator&) = delete;
    auto operator=(const Allocator&) -> Allocator& = delete;

    auto allocate(std::size_t size) -> void* {
        if (size == 0U) return nullptr;

        void* end = get_byte(m_begin) + sizeof(Header) + size;
        void* next = end;
        std::size_t free = 2U * alignof(Header);

        if (std::align(alignof(Header), sizeof(Header), next, free) == nullptr) {
            return nullptr;
        }

        std::size_t padding = static_cast<std::size_t>(get_byte(next) - get_byte(end));
        auto [current, previous] = SearchPolicy::template find<Allocator>(this, size + padding);

        if (current == nullptr) return nullptr;

        if (current->size >= size + padding + sizeof(Node) + 1U) {
            const std::size_t step = sizeof(Header) + size + padding;
            auto* node = get_node(get_byte(current) + step);
            node->size = current->size - step;
            node->next = current->next;
            current->next = node;
        } else {
            padding += current->size - size - padding;
        }

        if (previous == nullptr) {
            m_head = current->next;
        } else {
            previous->next = current->next;
        }

        auto* header = get_header(current);
        header->size = size + padding;
        return get_byte(current) + sizeof(Header);
    }

    void deallocate(void* ptr) {
        if (ptr == nullptr) return;

        auto* node = get_node(get_byte(ptr) - sizeof(Header));
        Node* previous = nullptr;
        Node* current = m_head;

        while (current != nullptr) {
            if (node < current) {
                node->next = current;
                if (previous == nullptr) {
                    m_head = node;
                } else {
                    previous->next = node;
                }
                merge(previous, node);
                return;
            }
            previous = current;
            current = current->next;
        }

        node->next = nullptr;
        if (previous == nullptr) {
            m_head = node;
        } else {
            previous->next = node;
        }
        merge(previous, node);
    }

    auto get_head() const -> Node* { return m_head; }

private:
    auto get_byte(void* ptr) const -> std::byte* {
        return static_cast<std::byte*>(ptr);
    }

    auto get_node(void* ptr) const -> Node* {
        return static_cast<Node*>(ptr);
    }

    auto get_header(void* ptr) const -> Header* {
        return static_cast<Header*>(ptr);
    }

    void merge(Node* previous, Node* node) {
        if (node->next != nullptr &&
            get_byte(node) + sizeof(Header) + node->size == get_byte(node->next)) {
            node->size += sizeof(Header) + node->next->size;
            node->next = node->next->next;
        }

        if (previous != nullptr &&
            get_byte(previous) + sizeof(Header) + previous->size == get_byte(node)) {
            previous->size += sizeof(Header) + node->size;
            previous->next = node->next;
        }
    }

    std::size_t m_size = 0U;
    void* m_begin = nullptr;
    Node* m_head = nullptr;
    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

void test_allocators() {
    // Тест FirstFit
    Allocator<FirstFitPolicy> allocator_ff(1024U);
    void* x = allocator_ff.allocate(16U);
    void* y = allocator_ff.allocate(16U);
    assert(x != nullptr && y != nullptr);
    allocator_ff.deallocate(y);
    allocator_ff.deallocate(x);
    void* z = allocator_ff.allocate(32U);
    assert(z == x);

    // Тест BestFit
    Allocator<BestFitPolicy> allocator_bf(1024U);
    x = allocator_bf.allocate(16U);
    y = allocator_bf.allocate(16U);
    assert(x != nullptr && y != nullptr);
    allocator_bf.deallocate(y);
    allocator_bf.deallocate(x);
    z = allocator_bf.allocate(32U);
    assert(z == x);

    std::cout << "All tests passed!\n";
}

static void benchmark_first_fit(benchmark::State& state) {
    constexpr std::size_t KB = 1U << 10;
    constexpr std::size_t MB = 1U << 20;
    std::vector<void*> blocks(KB, nullptr);

    for (auto _ : state) {
        Allocator<FirstFitPolicy> alloc(16U * MB);

        for (std::size_t i = 0U; i < KB; ++i) {
            blocks[i] = alloc.allocate(((i % 16U) + 1U) * 1024U);
        }

        for (std::size_t i = 0U; i < KB; i += 32U) {
            alloc.deallocate(blocks[i]);
        }

        for (std::size_t i = 0U; i < KB; i += 32U) {
            blocks[i] = alloc.allocate((((i + 7U) % 16U) + 1U) * 1024U);
        }

        for (std::size_t i = 0U; i < KB; ++i) {
            alloc.deallocate(blocks[i]);
        }

        benchmark::DoNotOptimize(blocks);
    }
}

static void benchmark_best_fit(benchmark::State& state) {
    constexpr std::size_t KB = 1U << 10;
    constexpr std::size_t MB = 1U << 20;
    std::vector<void*> blocks(KB, nullptr);

    for (auto _ : state) {
        Allocator<BestFitPolicy> alloc(16U * MB);

        for (std::size_t i = 0U; i < KB; ++i) {
            blocks[i] = alloc.allocate(((i % 16U) + 1U) * 1024U);
        }

        for (std::size_t i = 0U; i < KB; i += 32U) {
            alloc.deallocate(blocks[i]);
        }

        for (std::size_t i = 0U; i < KB; i += 32U) {
            blocks[i] = alloc.allocate((((i + 7U) % 16U) + 1U) * 1024U);
        }

        for (std::size_t i = 0U; i < KB; ++i) {
            alloc.deallocate(blocks[i]);
        }

        benchmark::DoNotOptimize(blocks);
    }
}

BENCHMARK(benchmark_first_fit);
BENCHMARK(benchmark_best_fit);

int main(int argc, char** argv) {
    test_allocators();

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}