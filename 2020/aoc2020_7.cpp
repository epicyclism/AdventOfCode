#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

#include "../compile-time-regular-expressions/single-header/ctre.hpp"

constexpr auto ln_rx = ctll::fixed_string{ R"(([a-z ]+) bags contain ([^\.]*)\.)" };
constexpr auto bg_rx = ctll::fixed_string{ R"((\d+) ([a-z]+ [a-z]+))" };

struct edge_t
{
    std::string colour_;
    int         cnt_;
};

using graph_t = std::map<std::string, std::vector<edge_t>>;

graph_t make_graph()
{
    graph_t g;
    std::string ln;
    while (std::getline(std::cin, ln))
    {
        if (auto [m, b, c] = ctre::match<ln_rx>(ln); m)
        {
            auto to = b.to_string();
            for (auto m : ctre::range<bg_rx>(c.to_view()))
            {
                auto cnt  = std::stoi(m.get<1>().to_string());
                auto from = m.get<2>().to_string();
                g[to].emplace_back(edge_t{from, cnt});
                g.try_emplace(from);
            }
        }
        else
            std::cout << "Line \"" << ln << "\" failed to parse.\n";
    }
    return g;
}

graph_t reverse_graph(graph_t const& g)
{
    graph_t go;
    for(auto al : g)
    {
        for( auto& f : al.second)
            go[f.colour_].emplace_back(edge_t{al.first, f.cnt_});
        go.try_emplace(al.first);
    }
    return go;
}

void dump_graph(graph_t const& g)
{
    for(auto p : g)
    {
        std::cout << '\"' << p.first << "\" :";
        for(auto b : p.second)
            std::cout << " \"" << b.colour_ << "\" (" << b.cnt_ << ')';
        std::cout << '\n';
    }
}

void pt1_worker(std::string const& u, graph_t const& g, std::set<std::string>& cc)
{
    for(auto& v : g.at(u))
    {
        cc.insert(v.colour_);        
        pt1_worker(v.colour_, g, cc );
    }
}

int pt1(graph_t const& g)
{
    std::set<std::string> collect_colours;
    pt1_worker("shiny gold", g, collect_colours);
    return collect_colours.size();
}

int pt2_worker(std::string const& u, graph_t const& g)
{
    int rv { 1 };
    for(auto& v : g.at(u))
        rv += v.cnt_ * pt2_worker(v.colour_, g);
    return rv;
}

int pt2(graph_t const& g)
{
    return pt2_worker("shiny gold", g) - 1;
}

int main()
{
    auto g = make_graph();
    dump_graph(reverse_graph(g));
    std::cout << "p1 = " << pt1(reverse_graph(g)) << '\n';
    std::cout << "p2 = " << pt2(g) << '\n';
}
