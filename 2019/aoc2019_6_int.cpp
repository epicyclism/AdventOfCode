#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <numeric>

size_t id_from_name(std::string const& name)
{
    static std::map<std::string, size_t> ids_;
    auto[it, b] = ids_.try_emplace(name, ids_.size());
    return (*it).second;
}

using graph_t = std::vector<std::vector<int>>;

void add_vertex(size_t id, graph_t& g)
{
    while ( g.size() <= id)
        g.push_back({});
}

void add_edge(size_t from, size_t to, graph_t& g)
{
    add_vertex(from, g);
    add_vertex(to, g);
    g[from].push_back(to);
}

graph_t get_input()
{
    graph_t g;
    std::string ln;
    while(std::getline(std::cin, ln))
    {
        auto p = ln.find(')');
        auto id_from = id_from_name(ln.substr(0, p));
        auto id_to   = id_from_name(ln.substr(p + 1));
        add_edge(id_from, id_to, g);
        add_edge(id_to, id_from, g);
    }
    return g;
}

std::vector<size_t> bfs(size_t id_from, graph_t const& g)
{
    std::vector<size_t> distances (g.size());
    std::vector<bool>   visited(g.size());
    std::queue<size_t> q;
    q.push(id_from);
    while( !q.empty())
    {
        auto u = q.front(); q.pop();
        visited[u] = true;
        for(auto v : g[u])
        {
            if( !visited[v])
            {
                distances[v]= distances[u] + 1;
                q.push(v);
            }
        }
    }
    return distances;
}

size_t pt1(graph_t const& g)
{
    auto d = bfs(id_from_name("COM"), g);
    return std::accumulate(d.begin(), d.end(), 0);
}

size_t pt2(graph_t const& g)
{
    auto d = bfs(id_from_name("YOU"), g);
    return d[id_from_name("SAN")] - 2;
}

int main()
{
    auto g = get_input();
    auto p1 = pt1(g);
    auto p2 = pt2(g);
    std::cout << "pt1 = " << p1 << '\n';    
    std::cout << "pt2 = " << p2 << '\n';    
}