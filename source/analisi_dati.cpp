#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include "polyline.h"
#include "mappa.h"
#include "analisi_dati.h"
#include "metodi.h"

using namespace std;

extern vector <PolyLine> polyline;  extern int n_poly;
extern map<string, vector<int>> subnets;
double sigma_MTSR, sigma_STIT;
int cnt_errore = 0;
vector <pair<int, double>> nl_poly;

//----------------------------------------------------------------------------------------------------
double CalcolaRappresentanza_media(const string &label)
{
  double all = 0, sub = 0;
  for (const auto &p : polyline) {
    if (p.n2c == 6) continue;
    all += (p.n_traj_FT + p.n_traj_TF)*p.length;
    if (find(subnets[label].begin(), subnets[label].end(), p.id_local) != subnets[label].end())
      sub += (p.n_traj_FT + p.n_traj_TF)*p.length;
  }
  return sub / all;
}
//----------------------------------------------------------------------------------------------------
double CalcolaSubnet(const string &label)
{
  double all = 0, sub = 0;
  for (const auto &p : polyline) {
    if (p.n2c == 6) continue;
    all += p.length;
    if (find(subnets[label].begin(), subnets[label].end(), p.id_local) != subnets[label].end())
      sub += p.length;
  }
  return sub / all;
}
//----------------------------------------------------------------------------------------------------




