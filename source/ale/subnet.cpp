#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <physycom/split.hpp>

#include "featsel.hpp"

using namespace std;
using namespace physycom;

//#define NO_NET_SPLIT

struct polystat
{
  enum
  {
    OFFSET_ID = 0,
    OFFSET_IDLOC = 1,
    OFFSET_NF = 2,
    OFFSET_NT = 3,
    OFFSET_FLUXFT = 4,
    OFFSET_FLUXTF = 5,
    OFFSET_FLUXTOT = 6,
    OFFSET_LENGTH = 7
  };

  int id, id_local, nF, nT;
  double length;
  map<string, int> flux;

  polystat() {};
  polystat(const string &line)
  {
    vector<string> tokens;
    split(tokens, line, ";", token_compress_off);
    id = stoi(tokens[OFFSET_ID]);
    id_local = stoi(tokens[OFFSET_IDLOC]);
    nF = stoi(tokens[OFFSET_NF]);
    nT = stoi(tokens[OFFSET_NT]);
    flux["ft"] = stoi(tokens[OFFSET_FLUXFT]);
    flux["tf"] = stoi(tokens[OFFSET_FLUXTF]);
    flux["tot"] = stoi(tokens[OFFSET_FLUXTOT]);
    length = stod(tokens[OFFSET_LENGTH]);
  }
};

struct polystatnet : polystat
{
//  p.id; p.id_local;   nodeF;  nodeT; p.n_traj_FT_IT; p.n_traj_TF_IT; TOT_IT; p.n_traj_FT_ST; p.n_traj_TF_ST; TOT_ST; length
//     2;          1; 4100832; 133532;              0;              0;      0;              0;              0;      0; 1013.58

  enum
  {
    OFFSET_ID = 0,
    OFFSET_IDLOC = 1,
    OFFSET_NF = 2,
    OFFSET_NT = 3,
    OFFSET_FLUXFT_IT = 4,
    OFFSET_FLUXTF_IT = 5,
    OFFSET_FLUXTOT_IT = 6,
    OFFSET_FLUXFT_ST = 7,
    OFFSET_FLUXTF_ST = 8,
    OFFSET_FLUXTOT_ST = 9,
    OFFSET_LENGTH = 10
  };

  polystatnet() {};
  polystatnet(const string &line)
  {
    vector<string> tokens;
    split(tokens, line, ";", token_compress_off);
    id = stoi(tokens[OFFSET_ID]);
    id_local = stoi(tokens[OFFSET_IDLOC]);
    nF = stoi(tokens[OFFSET_NF]);
    nT = stoi(tokens[OFFSET_NT]);
    flux["ft_it"] = stoi(tokens[OFFSET_FLUXFT_IT]);
    flux["tf_it"] = stoi(tokens[OFFSET_FLUXTF_IT]);
    flux["tot_it"] = stoi(tokens[OFFSET_FLUXTOT_IT]);
    flux["ft_st"] = stoi(tokens[OFFSET_FLUXFT_ST]);
    flux["tf_st"] = stoi(tokens[OFFSET_FLUXTF_ST]);
    flux["tot_st"] = stoi(tokens[OFFSET_FLUXTOT_ST]);
    flux["ft"] = flux["ft_it"] + flux["ft_st"];
    flux["tf"] = flux["tf_it"] + flux["tf_st"];
    flux["tot"] = flux["tot_it"] + flux["tot_st"];
    length = stod(tokens[OFFSET_LENGTH]);
  }
};

set<int> nodes;
map<int, map<int, int>> node_poly;
map<int, int> cid_lid, lid_cid;

template<typename poly_list>
void make_node_map(const poly_list &poly)
{
  for (const auto p : poly)
  {
    nodes.insert(p.nF);
    nodes.insert(p.nT);
    node_poly[p.nF][p.nT] = p.id_local;
    node_poly[p.nT][p.nF] = p.id_local;
  }

  int node_lid = 0;
  for (const auto &n : nodes)
  {
    lid_cid[node_lid] = n;
    cid_lid[n] = node_lid++;
  }
}

template<typename polystat_t>
vector<polystat_t> import_poly_stat(const string &filename)
{
  ifstream input(filename);
  if (!input)
  {
    cerr << "Unable to open input : " << filename << endl;
    exit(4);
  }

  string line;
  vector<polystat_t> poly;
  getline(input, line);   // skip header :  p.id;p.id_local;p.n2c;p.n_traj_FT;p.n_traj_TF; FT+TF
  getline(input, line);   // skip first fake line
  while (getline(input, line))
  {
    polystat_t p(line);
    poly.push_back(p);
  }
  make_node_map(poly);
  return poly;
}

int main()
{
#ifdef NO_NET_SPLIT
  auto poly = import_poly_stat<polystat>("../output/flussiSubnet.csv");
#else
  auto poly = import_poly_stat<polystatnet>("../output/flussiSubnet_IT_ST.csv");
#endif
  cout << "Poly parsed : " << poly.size() << endl;
  cout << "Node parsed : " << nodes.size() << endl;

  map<string, vector<int>> subnets;
  vector<string> sub_types({ "tot", "tot_it", "tot_st" /*, "ft_it", "tf_it", "ft_st", "tf_st"*/ });
  vector<double> sub_fractions({ 0.1, /*0.15, 0.2 , 0.4*/ });

  // allocate indices matrix
  int **ind; ind = new int*[poly.size()]; for (int i = 0; i < (int)poly.size(); ++i) ind[i] = new int[2];

  for (const auto &t : sub_types)
  {
    for (const auto &f : sub_fractions)
    {
      string label = t + "_" + to_string(int(f * 100));
      cout << "Processing : " << t << " @ " << f << endl;
      sort(poly.begin(), poly.end(), [t](const polystat &p1, const polystat &p2) { return p1.flux.at(t) > p2.flux.at(t); });
      for (int i = 0; i < (int)poly.size(); ++i)
      {
        ind[i][0] = cid_lid[poly[i].nF];
        ind[i][1] = cid_lid[poly[i].nT];
      }

      auto nodesel = FeatureSelection(ind, (int)poly.size(), int(f * nodes.size()), true, false);
      for (const auto &p : nodesel) subnets[label].push_back(node_poly[lid_cid[p.first]][lid_cid[p.second]]);
      sort(subnets[label].begin(), subnets[label].end());
      cout << "Selected poly : " << subnets[label].size() << endl;
    }
  }

  ofstream out("../output/subnet.txt");
  for (const auto &i : subnets)
  {
    out << i.first << "\t";
    for (const auto &p : i.second) out << p << "\t";
    out << endl;
  }
  out.close();

  cout << "Hit ENTER to quit."; cin.get();
  return 0;
}
