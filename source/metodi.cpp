#include "stdafx.h"
#include "polyline.h"
#include "nodo.h"
#include "metodi.h"


extern int  screen_w, screen_h;
double lat0, lon0, dlat, dlon, zoom_start;

extern vector <PolyLine> polyline;  extern int n_poly;
vector <Nodo> nodo;  int n_nodi;
vector<int> poly_eliminare;


//------------------------------------------------------------------------------------------------------
void Geometria() {

  dlat = 0.5*(LAT_MAX - LAT_MIN) / ZOOM_START; 	lat0 = 0.5*(LAT_MIN + LAT_MAX);
  dlon = 0.5*(LON_MAX - LON_MIN) / ZOOM_START;   	lon0 = 0.5*(LON_MIN + LON_MAX);

  if (DSLAT*dlat > DSLON*dlon) { dlon = (DSLAT / DSLON)*dlat*double(screen_w) / screen_h; }
  else { dlat = (DSLON / DSLAT)*dlon*double(screen_h) / screen_w; }

}
//----------------------------------------------------------------------------------------------------
void CreaNodi() {
  map<long long int, int> nodo_cid2id;

  for (int i = 1; i < n_poly; i++) {
    polyline[i].id_local = i;
    nodo_cid2id[polyline[i].cid_Fjnct] = 0;
    nodo_cid2id[polyline[i].cid_Tjnct] = 0;
  }
  int cnt = 1; for (auto &i : nodo_cid2id) i.second = cnt++;
  n_nodi = int(nodo_cid2id.size()) + 1;

  nodo.resize(n_nodi);
  for (int i = 1; i < n_poly; i++) {
    int id_nodoF = nodo_cid2id[polyline[i].cid_Fjnct];
    int id_nodoT = nodo_cid2id[polyline[i].cid_Tjnct];

    nodo[id_nodoF].add_link(id_nodoT, i);
    nodo[id_nodoT].add_link(id_nodoF, -i);
    nodo[id_nodoF].id_nodo = id_nodoF; nodo[id_nodoF].cid_nodo = polyline[i].cid_Fjnct;
    nodo[id_nodoT].id_nodo = id_nodoT; nodo[id_nodoT].cid_nodo = polyline[i].cid_Tjnct;
    nodo[id_nodoF].lat = polyline[i].points.front().lat;
    nodo[id_nodoF].lon = polyline[i].points.front().lon;
    nodo[id_nodoT].lat = polyline[i].points.back().lat;
    nodo[id_nodoT].lon = polyline[i].points.back().lon;
  }


  cout << "CreaNodi: n_nodi " << n_nodi << endl;
  map <long long int, int> cid2id;
  for (int i = 1; i < n_nodi; ++i) 	cid2id[nodo[i].cid_nodo] = i;
  for (int i = 1; i < n_poly; i++) {
    polyline[i].nodo_F = cid2id[polyline[i].cid_Fjnct];
    polyline[i].nodo_T = cid2id[polyline[i].cid_Tjnct];
  }
  cid2id.clear();
  //cerca le coppie di poly che uniscono gli stessi nodi
  for (int n = 1; n < n_nodi; n++)
    for (int i = 0; i < nodo[n].id_nodo_vicino.size() - 1; ++i)
      for (int j = i + 1; j < nodo[n].id_nodo_vicino.size(); ++j)
        if (nodo[n].id_nodo_vicino[i] == nodo[n].id_nodo_vicino[j])
        {
          if (polyline[abs(nodo[n].id_link[i])].length > polyline[abs(nodo[n].id_link[j])].length)  poly_eliminare.push_back(abs(nodo[n].id_link[i]));
          else poly_eliminare.push_back(abs(nodo[n].id_link[j]));
        }
  sort(poly_eliminare.begin(), poly_eliminare.end());
  poly_eliminare.erase(unique(poly_eliminare.begin(), poly_eliminare.end()), poly_eliminare.end());
  cout << "size poly da eliminare: " << poly_eliminare.size() << endl;
  int conta = 0;
  for (auto k = polyline.begin(); k < polyline.end(); ++k) {
    k->ok = true;
    for (auto j : poly_eliminare)
      if (j == k->id_local) { k->ok = false; break; }
  }
}

//----------------------------------------------------------------------------------------------------
FILE *my_fopen(char *fname, char *mode)
{
  FILE *fp;

  if ((fp = fopen(fname, mode)) == NULL)
  {
    fprintf(stderr, "errore - impossibile aprire il file %s in %s\n",
      fname, (mode[0] == 'r') ? "lettura" : "scrittura");
    exit(1);
  }
  return fp;
}
//----------------------------------------------------------------------------------------------------------------
