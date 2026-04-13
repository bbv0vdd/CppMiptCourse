#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

class VectorCapacityAnalyzer
{
public:
    void analyze(std::size_t elements)
    {
        std::vector<int> container;
        std::size_t prev_cap = container.capacity();
        
        for (std::size_t i = 0; i < elements; ++i)
        {
            container.push_back(static_cast<int>(i));
            
            if (container.capacity() != prev_cap)
            {
                Record r;
                r.current_size = container.size();
                r.previous = prev_cap;
                r.current = container.capacity();
                r.factor = prev_cap ? static_cast<double>(r.current) / prev_cap : 0.0;
                records.push_back(r);
                prev_cap = r.current;
            }
        }
    }
    
    double compute_factor() const
    {
        std::vector<double> factors;
        for (const auto& rec : records)
            if (rec.previous != 0)
                factors.push_back(rec.factor);
        
        if (factors.size() >= 3)
        {
            factors.erase(factors.begin());
            factors.pop_back();
        }
        
        return std::accumulate(factors.begin(), factors.end(), 0.0) / factors.size();
    }
    
    void show() const
    {
        std::cout << "Vector capacity changes:\n";
        for (const auto& rec : records)
        {
            std::cout << "size=" << std::setw(4) << rec.current_size
                      << " cap: " << std::setw(4) << rec.previous
                      << "->" << std::setw(4) << rec.current;
            if (rec.previous)
                std::cout << " factor=" << std::fixed << std::setprecision(3) << rec.factor;
            std::cout << '\n';
        }
        std::cout << "Growth factor estimate: " << std::fixed << std::setprecision(3) 
                  << compute_factor() << '\n';
    }
    
private:
    struct Record
    {
        std::size_t current_size = 0;
        std::size_t previous = 0;
        std::size_t current = 0;
        double factor = 0.0;
    };
    std::vector<Record> records;
};

class DequePageAnalyzer
{
public:
    void scan(std::size_t count)
    {
        std::deque<int> dq;
        for (std::size_t i = 0; i < count; ++i)
            dq.push_back(static_cast<int>(i));
        
        std::uintptr_t step = sizeof(int);
        std::size_t seg_start = 0;
        std::uintptr_t prev_addr = reinterpret_cast<std::uintptr_t>(&dq[0]);
        
        for (std::size_t i = 1; i < dq.size(); ++i)
        {
            std::uintptr_t curr_addr = reinterpret_cast<std::uintptr_t>(&dq[i]);
            if (curr_addr != prev_addr + step)
            {
                add_segment(dq, seg_start, i - 1);
                seg_start = i;
            }
            prev_addr = curr_addr;
        }
        add_segment(dq, seg_start, dq.size() - 1);
    }
    
    std::size_t page_size() const
    {
        if (segments.size() <= 2)
            return segments.front().count;
        
        std::vector<std::size_t> middle_counts;
        for (std::size_t i = 1; i + 1 < segments.size(); ++i)
            middle_counts.push_back(segments[i].count);
        
        auto p = std::minmax_element(middle_counts.begin(), middle_counts.end());
        return *p.first;
    }
    
    void report() const
    {
        std::cout << "Deque memory segments:\n";
        for (const auto& seg : segments)
        {
            std::cout << "[" << std::setw(4) << seg.start_idx
                      << "," << std::setw(4) << seg.end_idx
                      << "] cnt=" << std::setw(4) << seg.count
                      << " addr: 0x" << std::hex << seg.start_addr
                      << "..0x" << seg.end_addr << std::dec << '\n';
        }
        std::cout << "Page size estimate: " << page_size() << " elements\n";
    }
    
    void trace_insertions(std::size_t steps) const
    {
        std::deque<int> dq;
        std::cout << "Deque insertion trace:\n";
        
        for (std::size_t i = 0; i < steps; ++i)
        {
            dq.push_back(static_cast<int>(i));
            std::cout << "push(" << i << ") sz=" << dq.size()
                      << " addr=0x" << std::hex
                      << reinterpret_cast<std::uintptr_t>(&dq.back()) << std::dec;
            
            if (dq.size() >= 2)
            {
                std::uintptr_t prev = reinterpret_cast<std::uintptr_t>(&dq[dq.size() - 2]);
                std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(&dq.back());
                if (curr != prev + sizeof(int))
                    std::cout << " [NEW PAGE]";
            }
            std::cout << '\n';
        }
    }
    
private:
    struct SegmentInfo
    {
        std::size_t start_idx = 0;
        std::size_t end_idx = 0;
        std::size_t count = 0;
        std::uintptr_t start_addr = 0;
        std::uintptr_t end_addr = 0;
    };
    
    void add_segment(const std::deque<int>& dq, std::size_t start, std::size_t end)
    {
        SegmentInfo seg;
        seg.start_idx = start;
        seg.end_idx = end;
        seg.count = end - start + 1;
        seg.start_addr = reinterpret_cast<std::uintptr_t>(&dq[start]);
        seg.end_addr = reinterpret_cast<std::uintptr_t>(&dq[end]);
        segments.push_back(seg);
    }
    
    std::vector<SegmentInfo> segments;
};

int main()
{
    VectorCapacityAnalyzer vec_analyzer;
    vec_analyzer.analyze(512);
    vec_analyzer.show();
    
    std::cout << "\n";
    
    DequePageAnalyzer dq_analyzer;
    dq_analyzer.scan(1024);
    dq_analyzer.report();
    
    std::cout << "\n";
    dq_analyzer.trace_insertions(129);
    
    return 0;
}