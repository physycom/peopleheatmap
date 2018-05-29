#pragma once
#include <common.h>
#include "dati_local.h"

using namespace std;

//----------------------------------------------------------------------------
struct PointXY {
  double lon, lat;
  void set(double lon_, double lat_);
  void set(int ilon, int ilat);
  void stampa(void);
};
//----------------------------------------------------------------------------
struct Segmento {
  int id_poly;
  double dx, dy, length, s0;
  PointXY a, b;
  void set(int id_poly, double s0, PointXY a, PointXY b);
  double distanza(double x, double y);
};
//----------------------------------------------------------------------------
struct Fl2D {
  double FT, TF;
  Fl2D();
};
//----------------------------------------------------------------------------
struct PolyLine {
  string           name;
  double           length, weightFT, weightTF;
  int              nodo_F, nodo_T;
  int              id, id_local, frc, n2c, fow, kmh, lanes, oneway; // oneway=0,1,2,3 --- doppio senso,FT,TF,ZTL
  long long int    cid_poly, cid_Fjnct, cid_Tjnct;
  bool             ok = true, visibile = false;
  bool             path_visibile = false;
  int              bitFrame = 0;
  unsigned short   bitPatt = 0x000F;
  int              n_traj_TF, n_traj_FT;
  map <string, Fl2D> flussi_distinti; //mappa di dimensione 2: flusso_IT, flusso_ST
  double           diffST_IT = 0;
  double           diffMT_SR = 0;
  vector <PointXY> points, delta_points; int nPoints;
  vector <int>     FT_svolte, TF_svolte;
  PolyLine(void);
  void set(int id_, long long int cid_, vector <PointXY> punto_);
  void set(long long int cid_Fjnct_, long long int cid_Tjnct_, float meters_,
    int frc_, int n2c_, int fow_, int oneway_, int kmh_, int lanes_, string name_);
  void set(long long int cid_Fjnct_, long long int cid_Tjnct_);
  void Inverti();
  void clear(void);
  void CalcolaLength(void);
  void stampa(int i, FILE *fp);
  void stampa_punti(int i, FILE *fp);
  void stampa_pro(FILE *fp);
  void stampa_pnt(FILE *fp);
};
//----------------------------------------------------------------------------
struct PolyPro {
  int              id, oneway, n_car_TF, n_car_FT;
  long long int    cid;
  float            length, vmed_TF, vmed_FT, flow_TF, flow_FT, v85_TF, v85_FT;
};
//----------------------------------------------------------------------------
struct PolyPnt {
  int              id, nPoints; //ilat,ilon;
  long long int    cid;
};
//----------------------------------------------------------------------------
