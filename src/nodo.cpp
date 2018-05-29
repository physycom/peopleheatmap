#include <common.h>
#include "nodo.h"

//---------------------------------- Nodo ----------------------------
Nodo::Nodo(void) { id_nodo = 0; }
//---------------------------------- Nodo ----------------------------
Nodo::~Nodo(void) { id_nodo_vicino.clear(); id_link.clear(); }
//--------------------------------------------------------------------
void Nodo::reset(void) { id_nodo = 0; id_nodo_vicino.clear(); id_link.clear(); }
//--------------------------------------------------------------------
void Nodo::set(long long int cid_nodo, int id_nodo) {
  this->cid_nodo = cid_nodo;
  this->id_nodo = id_nodo;
}
//--------------------------------------------------------------------
void Nodo::add_link(int id_nv, int id_lv) {
  id_nodo_vicino.push_back(id_nv);
  id_link.push_back(id_lv);
}
//--------------------------------------------------------------------
void Nodo::remove_link(void) {
  id_nodo_vicino.pop_back();
  id_link.pop_back();
}
//--------------------------------------------------------------------
int  Nodo::n_link(void) { return int(id_link.size()); }
//----------------------------------------------------------------------------
void Nodo::stampa(FILE *fp1) {
  fprintf(fp1, "%15lld\t%d\t%d\n", cid_nodo, id_nodo, n_link());
  for (int i = 0; i < n_link(); i++)
    fprintf(fp1, "%d\t%d\n", id_link[i], id_nodo_vicino[i]);
}
//----------------------------------------------------------------------------
double Nodo::distanza(double lon, double lat) {
  double dx = DSLON*(this->lon - lon);
  double dy = DSLAT*(this->lat - lat);
  double ds2 = dx*dx + dy*dy;
  if (ds2 > 0) return sqrt(ds2);
  else          return 0.0;
}
//----------------------------------------------------------------------------
bool CompNodoVicino(const NodoVicino& a, const NodoVicino& b) { return (a.distanza < b.distanza); }
//----------------------------------------------------------------------------
