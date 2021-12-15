#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <map>
#include <queue>

struct arena_t
{
    std::vector<char> donut_;
    int sx_;
    std::map<std::string, std::pair<int, int>> vertices_; // name, outer pos in donut, inner pos in donut. pos 0 is not present.
};

arena_t get_arena()
{
    arena_t a;

    // read the input
    std::string ln;
    std::getline(std::cin, ln);
    auto sx = ln.length();
    std::cout << "sx = " << sx << '\n';
    a.donut_.reserve(sx * sx); // it's square
    a.donut_.assign(ln.begin(), ln.end());
    while(std::getline(std::cin, ln))
        a.donut_.insert(a.donut_.end(), ln.begin(), ln.end() );

    // parse out vertices
    for (int n = 0; n < a.donut_.size() - sx; ++n)
    {
        if (::isalpha(a.donut_[n]))
        {
            bool outer = true;
            int vp = 0;
            std::string nm;
            if (::isalpha(a.donut_[n + sx])) // vertical
            {
                nm = a.donut_[n];
                nm += a.donut_[n + sx];
                if (n < sx) // top
                    vp = n + 2 * sx;
                else
                if (n < a.donut_.size() / 2) // upper inner
                {
                    vp = n - sx;
                    outer = false;
                }
                else
                if (n < a.donut_.size() - 2 * sx) // lower inner
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
                nm = a.donut_[n];
                nm += a.donut_[n + 1];
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
                    a.vertices_[nm].first = vp;
                else
                    a.vertices_[nm].second = vp;
            }
        }
    }
    a.sx_ = sx;
    return a;
}

void print_vertices(arena_t const& a)
{
    for(auto& vp : a.vertices_)
        std::cout << vp.first << " : " << vp.second.first << ", " << vp.second.second << '\n';
}

using can_move_set = std::array<int, 4>;
can_move_set get_moves(arena_t const& a, int p)
{
    can_move_set cms;
    cms[0] = a.donut_[p - a.sx_] == '.' ? p - a.sx_ : -1;
    cms[1] = a.donut_[p + a.sx_] == '.' ? p + a.sx_ : -1;
    cms[2] = a.donut_[p - 1] == '.' ? p - 1 : -1;
    cms[3] = a.donut_[p + 1] == '.' ? p + 1 : -1;

    return cms;
}

std::vector<int> bfs(arena_t const& a, int s)
{
    std::vector <int> d(a.donut_.size(), -1);
    std::queue<size_t> q;
    q.push(s);
    d[s] = 0;
    while (!q.empty())
    {
        auto p = q.front(); q.pop();

        auto cms = get_moves(a, p);
        for (auto& v : cms)
        {
            if (v != -1 && (d[v] == -1))
            {
                d[v] = d[p] + 1;
                q.push(v);
            }
        }
    }

    return d;
}

struct edge_store_t
{
    int f_;
    int t_;
    int w_;
    edge_store_t(int f, int t, int w) : f_(f), t_(t), w_(w)
    {}
};

struct edge_store
{
    // convenience store for vertex names
    std::vector<std::string> vnm_;
    // list of derived weighted edges
    // the first vnm_.size vertex_ids are on the outside, the next on the inside
    // there are two redundant spaces since AA and ZZ aren't present internally.
    std::vector<edge_store_t> edges_;
};

edge_store build_edge_store(arena_t const& a)
{
    edge_store es;
    int vid_f = 0;
    for (auto& v : a.vertices_)
    {
        es.vnm_.emplace_back(v.first);
        auto d = bfs(a, v.second.first);
        int vid_t = 0;
        for (auto& e : a.vertices_)
        {
            if (d[e.second.first] > 0)
                es.edges_.emplace_back(vid_f, vid_t, d[e.second.first]);
            if (d[e.second.second] > 0)
                es.edges_.emplace_back(vid_f, vid_t + a.vertices_.size(), d[e.second.second]);
            ++vid_t;
        }
        d = bfs(a, v.second.second);
        auto vid_f2 { vid_f + a.vertices_.size()};
        vid_t = 0;
        for (auto& e : a.vertices_)
        {
            if (d[e.second.first] > 0)
                es.edges_.emplace_back(vid_f2, vid_t, d[e.second.first]);
            if (d[e.second.second] > 0)
                es.edges_.emplace_back(vid_f2, vid_t + a.vertices_.size(), d[e.second.second]);
            ++vid_t;
        }
        ++vid_f;
    }

    return es;
}

void print_edge_store(edge_store const& es)
{
    for(auto& e : es.edges_)
    {
        auto& fnm = es.vnm_[e.f_ % es.vnm_.size()];
        auto& tnm = es.vnm_[e.t_ % es.vnm_.size()];
        std::cout << fnm << " -> " << tnm << " (" << e.w_ << ")\n";
    }
}

struct edge_t
{
    int to_;
    int wt_;
};

using graph_t = std::vector<std::vector<edge_t>>;

void add_edge(int v1, int v2, int wt, graph_t& g)
{
    g[v1].push_back({v2, wt});
    g[v2].push_back({v1, wt});
}

std::vector<int> dijkstra(int from, graph_t const& g)
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

void print_graph(graph_t const& g)
{
    int n = 0;
    for(auto& al : g)
    {
        std::cout << n << " : " ;
        for(auto& t : al)
            std::cout << t.to_ << ' ';
        std::cout << '\n';
        ++n;
    }
}

int pt1(edge_store const& es)
{
    // install the edges in a graph
    int nv = es.vnm_.size(); // number of outside vertices
    graph_t g(nv * 2); // each named vertex represents two (apart from AA and ZZ) actual vertices
    for(auto& e : es.edges_)
        add_edge(e.f_, e.t_, e.w_, g);
    // make the part one connections, to link inside and outside versions of each vertex
    // assume AA is first and ZZ is last, with a count of N, then link 1, N + 1 -> N - 2, 2N - 2
    for ( int v = 1; v < nv - 1; ++v)
        add_edge(v, v + nv, 1, g);
    print_graph(g);
    // now do the dijkstra thing,source vertex is 0, target is nv - 1;
    auto d = dijkstra(0, g);

    return d[nv - 1];
}

int pt2(edge_store const& es)
{
    int nv = es.vnm_.size(); // number of outside vertices
    // install the base
    graph_t g(nv * 2 * 33); // each named vertex represents two (apart from AA and ZZ) actual vertices, account for 32 copies
    // repeat more times,
    for( int cnt = 0; cnt < 32; ++cnt)
    {
        int base_vertex = cnt * (nv * 2);
        // install a copy
        for(auto& e : es.edges_)
            add_edge(e.f_ + base_vertex, e.t_ + base_vertex, e.w_, g);
        // join inside of parent to outside of child
        for ( int v = 1; v < nv - 1; ++v)
            add_edge(v + base_vertex + nv, v + 2 * nv + base_vertex, 1, g);
    }
    // now do the dijkstra thing,source vertex is 0, target is nv - 1;
    auto d = dijkstra(0, g);

    return d[nv - 1];

}

int main()
{
    std::cout << "Reading input\n";
    auto a = get_arena();
    print_vertices(a);
    auto es = build_edge_store(a);
    print_edge_store(es);
    std::cout << "part 1 = " << pt1(es) << '\n';
    std::cout << "part 2 = " << pt2(es) << '\n';
}
