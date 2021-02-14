#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <queue>

#include <boost/container/flat_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

struct arena_t
{
    std::vector<char> donut_;
    size_t sx_;
    std::vector<std::pair<size_t, size_t>> vertices_;
    size_t in_;
    size_t out_;
    size_t point_to_vertex(size_t pt)
    {
        for (auto n = 0; n < vertices_.size(); ++n)
            if (vertices_[n].first == pt || vertices_[n].second == pt)
                return n;
        return -1;
    }
};

struct vn_t
{
    char nm_[2];
};

bool operator<(vn_t const& l, vn_t const& r)
{
    if (l.nm_[0] == r.nm_[0])
        return l.nm_[1] < r.nm_[1];
    return l.nm_[0] < r.nm_[0];
}

bool operator==(vn_t const& l, char const* r)
{
    return (l.nm_[0] == r[0]) && l.nm_[1] == r[1];
}

arena_t get_arena(std::vector<std::string> const& in)
{
    arena_t a;
    // two lines for vertices, each line two chars for vertex, donut, two chars, space, two chars, donut, two chars for vertex, two lines for vertices.
    size_t sx = in[0].length();
    a.donut_.reserve(sx * in.size());
    for(auto& ln : in)
        a.donut_.insert(a.donut_.end(), ln.begin(), ln.end() );

    // parse out vertices
    std::map<vn_t, std::pair<size_t, size_t>> mvn;
    for (size_t n = 0; n < a.donut_.size() - sx; ++n)
    {
        if (::isalpha(a.donut_[n]))
        {
            bool outer = true;
            size_t vp = 0;
            vn_t nm;
            if (::isalpha(a.donut_[n + sx])) // vertical
            {
                nm.nm_[0] = a.donut_[n];
                nm.nm_[1] = a.donut_[n + sx];
                if (n < sx) // top
                    vp = n + 2 * sx;
                else
                if (n / sx < in.size() / 2) // upper inner
                {
                    vp = n - sx;
                    outer = false;
                }
                else
                if (n / sx < in.size() - 3) // lower inner
                {
                    vp = n + 2 * sx;
                    outer = false;
                }
                else
                    vp = n - sx;
            }
            else
            if (::isalpha(a.donut_[n + 1])) // horizontal
            {
                nm.nm_[0] = a.donut_[n];
                nm.nm_[1] = a.donut_[n + 1];
                if (n % sx == 0) // left 
                    vp = n + 2;
                else
                if (n % sx < sx / 2) // left inner
                {
                    vp = n - 1;
                    outer = false;
                }
                else
                if (n % sx == sx - 2) // right
                    vp = n - 1;
                else
                {
                    vp = n + 2;
                    outer = false;
                }
            }
            if (vp)
            {
                if (outer)
                    mvn[nm].first = vp;
                else
                    mvn[nm].second = vp;
            }
        }
    }
    for (auto& v : mvn)
    {
        if (v.first == "AA")
            a.in_ = a.vertices_.size();
        else
        if (v.first == "ZZ")
            a.out_ = 2 * a.vertices_.size();
        std::cout << v.first.nm_[0] << v.first.nm_[1] << " -> " << a.vertices_.size() << "( " << v.second.first << ", " << v.second.second << ")\n";
        a.vertices_.push_back(v.second);
    }
    a.sx_ = sx;
    return a;
}

using namespace boost;

using graph_t = adjacency_list < vecS, vecS, undirectedS,
    no_property, property<edge_weight_t, size_t>>;

using vertex_t = graph_traits<graph_t>::vertex_descriptor;

using can_move_set = std::array<size_t, 4>;
can_move_set get_moves(arena_t const& a, size_t p)
{
    can_move_set cms;
    cms[0] = a.donut_[p - a.sx_] == '.' ? p - a.sx_ : -1;
    cms[1] = a.donut_[p + a.sx_] == '.' ? p + a.sx_ : -1;
    cms[2] = a.donut_[p - 1] == '.' ? p - 1 : -1;
    cms[3] = a.donut_[p + 1] == '.' ? p + 1 : -1;

    return cms;
}

std::vector<size_t> bfs(arena_t const& a, size_t s)
{
    std::vector <size_t> d(a.donut_.size());
    std::fill(d.begin(), d.end(), 0);
    std::queue<size_t> q;
    if(s != 0)
        q.push(s);
    while (!q.empty())
    {
        auto p = q.front(); q.pop();
        auto cms = get_moves(a, p);
        for (auto& v : cms)
        {
            if (v != -1 && (d[v] == 0))
            {
                d[v] = d[p] + 1;
                q.push(v);
            }
        }
    }
    return d;
}

void add_edge_(size_t b, size_t e, size_t w, graph_t& g)
{
    if (b == e)
        return;
    std::cout << "From " << b << " to " << e << ", weight " << w << '\n';
    add_edge(b, e, w, g);
}

struct edge_t
{
    size_t f_;
    size_t t_;
    size_t w_;
    edge_t(size_t f, size_t t, size_t w) : f_(f), t_(t), w_(w)
    {}
};

using edge_store = std::vector<edge_t>;

void build_graph(graph_t& rg, edge_store const& ve, size_t nv, size_t rnd, bool loop)
{
    if (loop)
    {
         for(size_t vct = 0; vct < nv; vct += 2)
            add_edge(vct, vct + 1, 1, rg);
    }
    else
    if(rnd > 0)
    {
        for (size_t vct = nv * (rnd - 1); vct < nv * rnd; vct += 2)
            add_edge(vct + 1, vct + nv, 1, rg);
    }
    auto vc = nv * rnd;
    for (auto& v : ve)
        add_edge(v.f_ + vc, v.t_ + vc, v.w_, rg);
}

std::pair<edge_store, size_t> build_edge_store(arena_t const& a)
{
    edge_store es;
    size_t vc = 0;
    vc = 0;
    for (auto& v : a.vertices_)
    {
        auto d = bfs(a, v.first);
        size_t ec = 0;
        for (auto& e : a.vertices_)
        {
            if (d[e.first])
                es.emplace_back(vc, ec, d[e.first]);
            if (d[e.second])
                es.emplace_back(vc, ec + 1, d[e.second]);
            ++ec;
            ++ec;
        }
        ec = 0;
        ++vc;
        d = bfs(a, v.second);
        for (auto& e : a.vertices_)
        {
            if (d[e.first])
                es.emplace_back(vc, ec, d[e.first]);
            if (d[e.second])
                es.emplace_back(vc, ec + 1, d[e.second]);
            ++ec;
            ++ec;
        }
        ++vc;
    }

    return { es, a.vertices_.size() * 2 };
//    return { es, vc - 1 };
}

std::vector<std::string> get_input()
{
    std::vector<std::string> rv;
    std::string ln;
    while (std::getline(std::cin, ln))
        rv.emplace_back(ln);

    return rv;
}

void pt1(arena_t a)
{
    auto[ es, ne] = build_edge_store(a);
    graph_t g;
    build_graph(g, es, ne, 0, true);
    std::vector<size_t> d(num_vertices(g));
    dijkstra_shortest_paths(g, a.in_,
        distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));
    std::cout << "pt1 " << d[a.out_] << '\n';
}

void pt2(arena_t a)
{
    auto[ es, ne] = build_edge_store(a);
    graph_t g;
    int n = 0;
    build_graph(g, es, ne, n, false);
    while(1)
    {
        ++n;
        build_graph(g, es, ne, n, false);
        std::vector<size_t> d(num_vertices(g));
        dijkstra_shortest_paths(g, a.in_,
            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));
        if (d[a.out_] != -1)
        {
            std::cout << "pt2 " << d[a.out_] << ", after " << n << " recursions\n";
            return;
        }
    }
}

int main()
{
    auto in = get_input();
    auto a = get_arena(in);
    pt1(a);
    pt2(a);
}
