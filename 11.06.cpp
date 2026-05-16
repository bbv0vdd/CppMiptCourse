#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/properties.hpp>

namespace TSP
{
    using WeightProperty = boost::property<boost::edge_weight_t, int>;
    using Graph = boost::adjacency_matrix<boost::undirectedS, boost::no_property, WeightProperty>;
    using Path = std::vector<std::size_t>;

    struct Solution
    {
        Path vertices;
        int totalCost{};
    };

    class CompleteGraph
    {
    public:
        CompleteGraph(std::size_t vertexCount, int minWeight, int maxWeight)
            : m_graph(vertexCount)
            , m_vertexCount(vertexCount)
        {
            validateCount(vertexCount);
            validateRange(minWeight, maxWeight);
            fillRandom(minWeight, maxWeight);
        }

        explicit CompleteGraph(const std::vector<std::vector<int>>& weights)
            : m_graph(weights.size())
            , m_vertexCount(weights.size())
        {
            validateMatrix(weights);
            fillFromMatrix(weights);
        }

        [[nodiscard]] Solution findOptimal() const
        {
            Path candidate(m_vertexCount);
            std::iota(candidate.begin(), candidate.end(), std::size_t{});

            Solution best;
            best.vertices = candidate;
            best.totalCost = std::numeric_limits<int>::max();

            constexpr std::ptrdiff_t fixedStart = 1;
            auto permBegin = std::next(candidate.begin(), fixedStart);

            do
            {
                const int cost = computeCycleCost(candidate);
                if (cost < best.totalCost)
                {
                    best.vertices = candidate;
                    best.totalCost = cost;
                }
            }
            while (std::next_permutation(permBegin, candidate.end()));

            return best;
        }

        [[nodiscard]] int computeCycleCost(const Path& path) const
        {
            validatePath(path);

            int total{};
            for (std::size_t i{}; i + 1 < path.size(); ++i)
            {
                total += getWeight(path[i], path[i + 1]);
            }
            total += getWeight(path.back(), path.front());

            return total;
        }

        void printWeightMatrix(std::ostream& out) const
        {
            constexpr int selfWeight = 0;
            constexpr int width = 4;

            out << "weighted adjacency matrix\n";
            for (std::size_t i{}; i < m_vertexCount; ++i)
            {
                for (std::size_t j{}; j < m_vertexCount; ++j)
                {
                    const int val = (i == j) ? selfWeight : getWeight(i, j);
                    out << std::setw(width) << val;
                }
                out << '\n';
            }
        }

        void printIncidenceMatrix(std::ostream& out) const
        {
            constexpr int notIncident = 0;
            constexpr int incident = 1;
            constexpr int width = 3;

            std::vector<Graph::edge_descriptor> edges;
            const auto edgeRange = boost::edges(m_graph);
            std::copy(edgeRange.first, edgeRange.second, std::back_inserter(edges));

            out << "incidence matrix\n";
            for (std::size_t v{}; v < m_vertexCount; ++v)
            {
                for (const auto& e : edges)
                {
                    const auto src = static_cast<std::size_t>(boost::source(e, m_graph));
                    const auto tgt = static_cast<std::size_t>(boost::target(e, m_graph));
                    const int val = (v == src || v == tgt) ? incident : notIncident;
                    out << std::setw(width) << val;
                }
                out << '\n';
            }
        }

        static void printSolution(const Solution& sol, std::ostream& out)
        {
            constexpr char arrow[] = " -> ";
            out << "best path\n";
            for (const auto v : sol.vertices)
            {
                out << v << arrow;
            }
            out << sol.vertices.front() << '\n';
            out << "total cost\n" << sol.totalCost << '\n';
        }

    private:
        [[nodiscard]] int getWeight(std::size_t src, std::size_t tgt) const
        {
            const auto edge = boost::edge(src, tgt, m_graph);
            if (!edge.second) throw std::logic_error("Missing edge.");
            return getWeight(edge.first);
        }

        [[nodiscard]] int getWeight(Graph::edge_descriptor edge) const
        {
            const auto weightMap = boost::get(boost::edge_weight, m_graph);
            return boost::get(weightMap, edge);
        }

        void fillRandom(int minW, int maxW)
        {
            std::random_device rd;
            std::default_random_engine engine(rd());
            std::uniform_int_distribution<int> dist(minW, maxW);

            for (std::size_t i{}; i < m_vertexCount; ++i)
            {
                for (std::size_t j = i + 1; j < m_vertexCount; ++j)
                {
                    addEdge(i, j, dist(engine));
                }
            }
        }

        void fillFromMatrix(const std::vector<std::vector<int>>& weights)
        {
            for (std::size_t i{}; i < m_vertexCount; ++i)
            {
                for (std::size_t j = i + 1; j < m_vertexCount; ++j)
                {
                    addEdge(i, j, weights[i][j]);
                }
            }
        }

        void addEdge(std::size_t src, std::size_t tgt, int weight)
        {
            const auto result = boost::add_edge(src, tgt, WeightProperty(weight), m_graph);
            if (!result.second) throw std::logic_error("Failed to add edge.");
        }

        void validatePath(const Path& path) const
        {
            if (path.size() != m_vertexCount)
            {
                throw std::invalid_argument("Bad path size.");
            }
        }

        static void validateCount(std::size_t n)
        {
            constexpr std::size_t MIN = 2;
            if (n < MIN) throw std::invalid_argument("Too few vertices.");
        }

        static void validateRange(int minW, int maxW)
        {
            if (maxW < minW) throw std::invalid_argument("Bad weight range.");
        }

        static void validateMatrix(const std::vector<std::vector<int>>& mat)
        {
            const std::size_t n = mat.size();
            validateCount(n);

            for (const auto& row : mat)
            {
                if (row.size() != n) throw std::invalid_argument("Bad matrix shape.");
            }

            for (std::size_t i{}; i < n; ++i)
            {
                for (std::size_t j{}; j < n; ++j)
                {
                    if (mat[i][j] != mat[j][i])
                    {
                        throw std::invalid_argument("Matrix is not symmetric.");
                    }
                }
            }
        }

        Graph m_graph;
        std::size_t m_vertexCount{};
    };
}

namespace Tests
{
    using namespace TSP;

    bool hasUniqueVertices(Path path, std::size_t count)
    {
        std::sort(path.begin(), path.end());
        Path expected(count);
        std::iota(expected.begin(), expected.end(), std::size_t{});
        return path == expected;
    }

    void run()
    {
        constexpr int ZERO = 0;
        constexpr int CHEAP = 1;
        constexpr int EXPENSIVE = 10;
        constexpr int EXPECTED_COST = 13;

        const std::vector<std::vector<int>> weights{
            {ZERO, CHEAP, EXPENSIVE, EXPENSIVE},
            {CHEAP, ZERO, CHEAP, EXPENSIVE},
            {EXPENSIVE, CHEAP, ZERO, CHEAP},
            {EXPENSIVE, EXPENSIVE, CHEAP, ZERO}
        };

        const CompleteGraph graph(weights);
        const auto solution = graph.findOptimal();

        assert(solution.totalCost == EXPECTED_COST);
        assert(graph.computeCycleCost(solution.vertices) == solution.totalCost);
        assert(hasUniqueVertices(solution.vertices, weights.size()));
    }
}

int main()
{
    constexpr std::size_t VERTICES = 10;
    constexpr int MIN_WEIGHT = 1;
    constexpr int MAX_WEIGHT = 10;

    Tests::run();
    std::cout << "tests passed\n\n";

    const TSP::CompleteGraph graph(VERTICES, MIN_WEIGHT, MAX_WEIGHT);
    const auto solution = graph.findOptimal();

    graph.printWeightMatrix(std::cout);
    std::cout << '\n';

    graph.printIncidenceMatrix(std::cout);
    std::cout << '\n';

    TSP::CompleteGraph::printSolution(solution, std::cout);
}