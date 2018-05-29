#include "stdafx.h"
#include "dati_local.h"
#include "polyline.h"

using namespace std;

//----------------------------------------------------------------------------------------
void PointXY::set(double lon_, double lat_) {
  lon = lon_; lat = lat_;
}
//----------------------------------------------------------------------------------------
void PointXY::set(int ilon, int ilat) {
  lon = 1.0E-6*double(ilon); lat = 1.0E-6*double(ilat);
}
//----------------------------------------------------------------------------------------
void PointXY::stampa(void) {
  fprintf(stdout, " %8.3lf %8.3lf \n", lon, lat);
}

//----------------------------------------------------------------------------------------
void Segmento::set(int id_poly, double s0, PointXY a, PointXY b) {
  this->id_poly = id_poly;
  this->s0 = s0;
  this->a = a;
  this->b = b;
  dx = DSLON*(b.lon - a.lon);   // calcolo versori
  dy = DSLAT*(b.lat - a.lat);
  length = sqrt(dx*dx + dy*dy);
  if (length > 0.01) { dx /= length; dy /= length; }
  else {
    cout << " id_poly= " << id_poly << " arc length= " << length << " in metri " << endl;
    cout << fixed << setprecision(6);
    cout << a.lat << "   " << a.lon << endl;
    cout << b.lat << "   " << b.lon << endl;
    PAU;
  }
}
//----------------------------------------------------------------------------------------
double Segmento::distanza(double lon, double lat) {  // in metri

  double dxa = DSLON*(lon - a.lon); double dya = DSLAT*(lat - a.lat);
  double ds = dxa*dx + dya*dy; double distanza;

  if (ds < 0) distanza = sqrt(dxa*dxa + dya*dya);
  else if (ds > length) {
    double dxb = DSLON*(lon - b.lon); double dyb = DSLAT*(lat - b.lat); distanza = sqrt(dxb*dxb + dyb*dyb);
  }
  else distanza = fabs(dxa*dy - dya*dx);

  return distanza;
}
//----------------------------------------------------------------------------------------
Fl2D::Fl2D() {
  this->FT = 0;
  this->TF = 0;
}
//----------------------------------------------------------------------------------------
PolyLine::PolyLine(void) {

  n_traj_TF = n_traj_FT = 0;

  clear();
}
//----------------------------------------------------------------------------------------
void PolyLine::clear(void) {
  points.clear(); delta_points.clear();
  //if( s != NULL) delete []s;
  visibile = false; nPoints = 0;
  name = "__";
}
//----------------------------------------------------------------------------------------
void PolyLine::set(int id, long long int cid, vector <PointXY> punto) {
  this->id = id;
  this->cid_poly = cid;
  points = punto;
  nPoints = int(points.size());
  //calcola_delta_points();
}
//----------------------------------------------------------------------------------------
void PolyLine::set(long long int cid_Fjnct, long long int cid_Tjnct, float meters,
  int frc_, int n2c_, int fow_, int oneway_, int kmh_, int lanes_, string name_) {
  this->cid_Fjnct = cid_Fjnct;
  this->cid_Tjnct = cid_Tjnct;
  this->length = meters;
  frc = frc_; n2c = n2c_; fow = fow_; kmh = kmh_; oneway = oneway_; lanes = lanes_; name = name_;
}
//----------------------------------------------------------------------------------------
void PolyLine::set(long long int cid_Fjnct, long long int cid_Tjnct) {
  this->cid_Fjnct = cid_Fjnct;
  this->cid_Tjnct = cid_Tjnct;
}
//----------------------------------------------------------------------------------------
void PolyLine::CalcolaLength(void) {
  double sum = 0.0;
  for (int k = 0; k < nPoints - 1; k++) {
    double dx = DSLON*(points[k + 1].lon - points[k].lon);
    double dy = DSLAT*(points[k + 1].lat - points[k].lat);
    double ds = sqrt(dx*dx + dy*dy);
    sum += ds;
  }
  length = sum;
  if (n2c == 6) weightFT = weightTF = length;
  else         weightFT = weightTF = length;
}
//----------------------------------------------------------------------------------------
void PolyLine::stampa(int i, FILE *fp1) {
  fprintf(fp1, " %8d %4d %10.2f %4d %4d %4d %8d %d %lld %s\n",
    i, nPoints, length, frc, n2c, fow, kmh, oneway, cid_poly, name.c_str());
  //fprintf(fp1," %8d %8d %4d %10.2f %4d %4d %4d %8d %d %lld %lld %lld %s\n",
  //		i,id_inizio,nPoints,length,frc,n2c,fow,kmh,oneway,
  //		cid,id_nodo_O,id_nodo_D,name.c_str() );
}
//----------------------------------------------------------------------------
void PolyLine::stampa_punti(int i, FILE *fp1) {
  for (int j = 0; j < nPoints; j++) fprintf(fp1, " %8d %8d %12.6lf %12.6lf\n", i, j, points[j].lon, points[j].lat);
}
//----------------------------------------------------------------------------
void PolyLine::stampa_pro(FILE *fp1) {
  fprintf(fp1, "%15lld %15lld %15lld %8.1f %2d %2d %2d %2d %3d %2d %s\n",
    cid_poly, cid_Fjnct, cid_Tjnct, length, frc, n2c, fow, oneway, kmh, lanes, name.c_str());
}
//----------------------------------------------------------------------------
void PolyLine::stampa_pnt(FILE *fp1) {
  fprintf(fp1, "%lld %d %d\n", cid_poly, id, nPoints);
  for (int i = 0; i < nPoints; i++) fprintf(fp1, "%8.0lf %8.0lf\n", points[i].lat*1e6, points[i].lon*1e6);
}
//----------------------------------------------------------------------------
void PolyLine::Inverti() {
  vector <PointXY> pw;
  for (vector<PointXY>::reverse_iterator rit = points.rbegin(); rit != points.rend(); ++rit) pw.push_back(*rit);
  points = pw;
}
//----------------------------------------------------------------------------

