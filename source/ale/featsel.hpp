/* Copyright (C) Nico Curti, Alessandro Fabbri - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Nico Curti <nico.curti2@unibo.it>
* Alessandro Fabbri <alessandro.fabbri27@unibo.it>
* Semptember 2017
*/


#ifndef __UTILS_FEATURESEL_HPP__
#define __UTILS_FEATURESEL_HPP__

#include <vector>
#include <string>
#include <set>
#include <boost/function.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/iteration_macros.hpp>

#define ENABLE_PERF false

typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_color_t, int>, boost::property<boost::edge_index_t, int>> Graph;
using V = Graph::vertex_descriptor;
using Filtered = boost::filtered_graph<Graph, boost::keep_all, boost::function<bool(V)> >;

template<typename T>
std::vector<std::pair<int, int>> FeatureSelection(T **Ig, const int &n_link, const int &min_size, bool pruning = true, bool merged = true)
{
  Graph G;
  std::set<V> removed_set;
  Filtered Signature(G, boost::keep_all{}, [&](V v) { return removed_set.end() == removed_set.find(v); });
  int L = 0, Ncomp, leave, i = 0;
  int max_key;
  std::vector<int> components, core;
  std::vector<std::pair<int, int>> sub;

  while (i < n_link)
  {
    boost::add_edge(Ig[i][0], Ig[i][1], G);

    while (pruning)
    {
      leave = 0;
      BGL_FORALL_VERTICES(v, Signature, Filtered)
        if (boost::in_degree(v, Signature) < 2)
        {
          removed_set.insert(v);
          ++leave;
        }
      if (leave == 0)
        break;
    }

    if (num_vertices(G) - removed_set.size())
    {
      components.resize(num_vertices(G));
      Ncomp = boost::connected_components(Signature, &components[0]);
      if (merged)
      {
        BGL_FORALL_VERTICES(v, Signature, Filtered)
          if (boost::in_degree(v, Signature))
            core.push_back((unsigned int)v);
      }
      else
      {
        std::map<unsigned int, unsigned int> size;
        for (auto &j : components)
          ++size[j];
        max_key = std::max_element(std::begin(size), std::end(size), [](const decltype(size)::value_type & p1, const decltype(size)::value_type & p2) {return p1.second < p2.second; })->first;
        BGL_FORALL_VERTICES(v, Signature, Filtered)
          if (components[v] == max_key)
            core.push_back((unsigned int)v);
      }
      L = (int)core.size();
    }

    if (L >= min_size)
    {
      BGL_FORALL_EDGES(l, Signature, Filtered)
      {
        if( components[l.m_source] == max_key && components[l.m_target] == max_key )
          sub.push_back(std::make_pair((int)l.m_source, (int)l.m_target));
      }
      break;
    }
    components.resize(0);
    ++i;
    removed_set.clear();
    core.resize(0);
  }

  return sub;
}

#endif // __UTILS_FEATURESEL_HPP__
