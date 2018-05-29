#pragma once

//----------------------------------------------------------------------------  
struct Nodo {
  Nodo(void);
  ~Nodo(void);
  long long int cid_nodo;
  int id_nodo;
  double lon, lat;
  vector <int> id_nodo_vicino, id_link;
  void reset(void);
  void set(long long int cid_nodo, int id_nodo);
  void add_link(int id_nv, int id_l);
  void remove_link(void);
  int n_link();
  double distanza(double lon, double lat);
  void stampa(FILE *fp1);
};
//---------------------------------------------------------------------
struct NodoVicino {
  int id_nodo;
  double distanza;
};
//---------------------------------------------------------------------
bool CompNodoVicino(const NodoVicino& a, const NodoVicino& b);
