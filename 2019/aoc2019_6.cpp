#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <numeric>

using graph_t = std::map<std::string, std::vector<std::string>>;

void add_edge(std::string const& from, std::string const& to, graph_t& g)
{
    g[from].push_back(to);
    g[to].push_back(from);
}

graph_t get_input()
{
    graph_t g;
    std::string ln;
    while(std::getline(std::cin, ln))
    {
        auto p = ln.find(')');
        add_edge(ln.substr(0, p), ln.substr(p + 1), g);
    }
    return g;
}

void print_graph(graph_t const& g)
{
    for(auto& r : g)
    {
        std::cout << r.first << " : " ;
        for(auto& v : r.second)
            std::cout << v << ' ';
        std::cout << '\n';
    }
    std::cout << '\n';
}

auto bfs(std::string const& src, graph_t const& g)
{
    std::map<std::string, size_t> distances;
    std::queue<std::string>       q;
    q.push(src);
    distances[src] = 0;
    while( !q.empty())
    {
        auto u = q.front(); q.pop();
        for(auto& v : g.at(u))
        {
            if( !distances.contains(v))
            {
                distances[v] = distances[u] + 1;
                q.push(v);
            }
        }
    }
    return distances;
}

size_t pt1(graph_t const& g)
{
    auto d = bfs("COM", g);
    return std::accumulate(d.begin(), d.end(), 0, [](auto s, auto& v){  return s + v.second;});
}

size_t pt2(graph_t const& g)
{
    auto d = bfs("YOU", g);
    return d["SAN"] - 2;
}

int main()
{
    auto g = get_input();
    print_graph(g);
    std::cout << "pt1 = " << pt1(g) << '\n';    
    std::cout << "pt2 = " << pt2(g) << '\n';    
}