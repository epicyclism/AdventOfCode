#include <iostream>
#include <compare>
#include <vector>
#include <limits>
#include <algorithm>

struct pt
{
    int x_;
    int y_;
    auto operator<=>(pt const&) const = default;
};

constexpr int rocky {0};
constexpr int wet {1};
constexpr int narrow {2};

constexpr int stride = 24;

struct cave_system
{
private:
    std::vector<int> sys_;
    const pt target_;
    const int depth_;

    int erosion_level(pt const& p)
    {
        int el = sys_[p.x_ + p.y_ * stride];
        if( el == -1)
        {
            int gi {0};
            if( p.x_ == 0)
                gi = p.y_ * 48271;
            else
            if(p.y_ == 0)
                gi = p.x_ * 16807;
            else
            if( p != target_)
                gi = erosion_level({p.x_ - 1, p.y_}) *
                        erosion_level({p.x_, p.y_ - 1});
            el = (gi + depth_) % 20183;
            sys_[p.x_ + p.y_ * stride] = el;
        }
        return el;
    }
public:
    cave_system(pt const& t, int d) : target_ {t}, depth_{d}, sys_(stride * 1024, -1)
    {}
    int type(pt const& p)
    {
        return erosion_level(p) % 3;
    }
};

constexpr int test_depth {510};
constexpr pt  test_target {10, 10};
constexpr int depth {7740};
constexpr pt  target {12, 763};

void test()
{
    cave_system cs { test_target, test_depth};
    std::cout << "el {  0,  0 } = " << cs.type({0, 0}) << '\n';
    std::cout << "el {  1,  0 } = " << cs.type({1, 0}) << '\n';
    std::cout << "el {  0,  1 } = " << cs.type({0, 1}) << '\n';
    std::cout << "el {  1,  1 } = " << cs.type({1, 1}) << '\n';
    std::cout << "el { 10, 10 } = " << cs.type({10, 10}) << '\n';
}

int pt1(pt const& t, int d)
{
    cave_system cs { t, d};
    int rv {0};
    for(int x = 0; x <= t.x_; ++x)
        for(int y = 0; y <= t.y_; ++y)
            rv += cs.type({x, y});
    
    return rv;
}

constexpr int neither {0};
constexpr int climbing {1};
constexpr int torch {2};

using vertex_t = int;
using edge_t  = struct{ int wt_; int to_;};
using graph_t = std::vector<std::vector<edge_t>>;

vertex_t vertex_id_from_region_tool(pt p, int tool)
{
    return (p.y_ * stride + p.x_) * 3 + tool;
}

void add_vertex(vertex_t id, graph_t& g)
{
    while( g.size() <= id)
        g.push_back({});
}

void add_edge(vertex_t from, vertex_t to, int weight, graph_t& g)
{
    add_vertex(from, g);
    add_vertex(to, g);
    g[from].push_back({weight, to});
}

// a region has three vertices, representing each 'tool',
// and edges to represent changing between each (7 min).
// the edge out of each vertex represents crossing this region
// and entering theh next (1min).
//
void install_region( pt p, graph_t& g)
{
    auto v_neither  = vertex_id_from_region_tool(p, neither);
    auto v_climbing = vertex_id_from_region_tool(p, climbing);
    auto v_torch    = vertex_id_from_region_tool(p, torch);
    // vertex for each tool
    add_vertex(v_neither, g);
    add_vertex(v_climbing, g);
    add_vertex(v_torch, g);    
    // tool swaps
    add_edge(v_neither, v_climbing, 7, g);
    add_edge(v_neither, v_torch, 7, g);
    add_edge(v_climbing, v_neither, 7, g);
    add_edge(v_climbing, v_torch, 7, g);
    add_edge(v_torch, v_neither, 7, g);
    add_edge(v_torch, v_climbing, 7, g);
}

bool cross_region( int type, int tool)
{
    switch(type)
    {
        case rocky:
            return tool != neither;
        case wet:
            return tool != torch ;
        case narrow:
            return tool != climbing;
        default:
            return false;
    }
}

graph_t build_graph (pt const& t, int d)
{
    graph_t g;
    // install the regions
    for( int y = 0; y < t.y_ + 16; ++y)
        for(int x = 0; x < stride; ++x)
            install_region({x, y}, g);
    // connect them together
    cave_system cs { t, d};
    for( int y = 0; y < t.y_ + 16; ++y)
        for(int x = 0; x < stride - 1; ++x)
        {
            for( int tool = 0; tool < 3; ++tool)
            {
                int type = cs.type({x, y});
                if( cross_region(type, tool))
                {
                    if( x > 0) // left
                        add_edge(vertex_id_from_region_tool({x, y}, tool), vertex_id_from_region_tool({x - 1, y}, tool), 1, g);
                    // right
                    add_edge(vertex_id_from_region_tool({x, y}, tool), vertex_id_from_region_tool({x + 1, y}, tool), 1, g);
                    if ( y > 0) // up
                        add_edge(vertex_id_from_region_tool({x, y}, tool), vertex_id_from_region_tool({x, y - 1}, tool), 1, g);
                    // down
                    add_edge(vertex_id_from_region_tool({x, y}, tool), vertex_id_from_region_tool({x, y + 1}, tool), 1, g);
                }
            }
        }
    return g;
}

void print( graph_t const& g)
{
    int v = 0;
    for(auto& al : g)
    {
        std::cout << v << " : ";
        for(auto& e : al)
        {
            std::cout << "{ " << e.to_ << ", " << e.wt_ << " }";
        }
        std::cout << '\n';
        ++v;
    }
}

std::vector<int> dijkstra(vertex_t from, graph_t const& g)
{
    std::vector<int> d(g.size(), std::numeric_limits<int>::max());
    std::vector<bool> inq(g.size(), true);
    // find th4 offset o of the min value of 'd' for which inq[o] is true
    // numeric_limits max if none
    auto find_min = [&]() -> int
    {
        auto m = std::numeric_limits<int>::max();
        auto mx = std::numeric_limits<int>::max();
        for(int n = 0; n < inq.size(); ++n)
            if( inq[n] && d[n] < mx)
            {
                m = n;
                mx = d[n];
            }
        return m;
    };
    auto u = from;
    d[u] = 0;
    while(u != std::numeric_limits<int>::max())
    {
        inq[u] = false;
        for( auto v : g[u])
        {
            if(inq[v.to_] && (d[v.to_] > d[u] + v.wt_))
                d[v.to_] = d[u] + v.wt_;
        }
        u = find_min();
    }
    return d;
}

int pt2(pt tgt, int depth)
{
    auto g = build_graph(tgt, depth);
    auto d = dijkstra(vertex_id_from_region_tool({0, 0}, torch), g);
    return d[vertex_id_from_region_tool(tgt, torch)];
}

int main()
{
    auto p1t = pt1(test_target, test_depth);
    std::cout << "pt1 (test) = " << p1t << '\n';
    auto p1 = pt1(target, depth);
    std::cout << "pt1        = " << p1 << '\n';
    auto p2t = pt2(test_target, test_depth);
    std::cout << "pt2 (test) = " << p2t << '\n';
    auto p2 = pt2(target, depth);
   std::cout << "pt2        = " << p2 << '\n';
}