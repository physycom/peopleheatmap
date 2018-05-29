#include "stdafx.h"
#include "polyline.h"
#include "nodo.h"
#include "mappa.h"

extern vector <PolyLine> polyline;  extern int n_poly;
extern vector <Nodo> nodo;  extern int n_nodi;

vector <Segmento> segmento;

// il verso della polilinea è definito da F(ront) a T(ail) dove il punto[0] è il front

//----------------------------------------------------------------------------
struct Elenco { vector <int> nodo_id,segmento_id,traj_end,traj_start; };
//----------------------------------------------------------------------------

Elenco **A; int jmax,imax; // A[jmax][imax]
double ds_lat, ds_lon, c_ris1, c_ris2; 

//------------------------------------------------------------------------------------------------------
int A_put_nodo(int n)
{
	double x, y; int n_nodi_put = 0;
	x = nodo[n].lon; y = nodo[n].lat;

	int ia = int((x - LON_MIN) / ds_lon);
	int ja = int((y - LAT_MIN) / ds_lat);
	int i0 = (ia - 1 > 0 ? ia - 1 : 0);  int i1 = (ia + 2 < imax ? ia + 2 : imax);
	int j0 = (ja - 1 > 0 ? ja - 1 : 0);  int j1 = (ja + 2 < jmax ? ja + 2 : jmax);

	for (int j = j0; j<j1; j++) {
		double lat_c = LAT_MIN + (j + 0.5)*ds_lat;
		double dya = DSLAT*(y - lat_c);
		for (int i = i0; i<i1; i++) {
			double lon_c = LON_MIN + (i + 0.5)*ds_lon;
			double dxa = DSLON*(x - lon_c);
			if (dxa*dxa + dya*dya < c_ris2) {
				A[j][i].nodo_id.push_back(n); n_nodi_put++;
			}
		}
	}
	return n_nodi_put;
}
//------------------------------------------------------------------------------------------------------
int A_put_segmento(int n) {

	int tw, n_seg_put = 0;
	int ia = int((segmento[n].a.lon - LON_MIN) / ds_lon);
	int ja = int((segmento[n].a.lat - LAT_MIN) / ds_lat);
	int ib = int((segmento[n].b.lon - LON_MIN) / ds_lon);
	int jb = int((segmento[n].b.lat - LAT_MIN) / ds_lat);
	if (ib<ia) { tw = ia; ia = ib; ib = tw; }         if (jb<ja) { tw = ja; ja = jb; jb = tw; }
	int i0 = (ia - 1 > 0 ? ia - 1 : 0);  int i1 = (ib + 2 < imax ? ib + 2 : imax);
	int j0 = (ja - 1 > 0 ? ja - 1 : 0);  int j1 = (jb + 2 < jmax ? jb + 2 : jmax);

	for (int j = j0; j<j1; j++) {
		double lat_c = LAT_MIN + (j + 0.5)*ds_lat;
		for (int i = i0; i<i1; i++) {
			double lon_c = LON_MIN + (i + 0.5)*ds_lon;
			if (segmento[n].distanza(lon_c, lat_c) < c_ris1) {
				A[j][i].segmento_id.push_back(n); n_seg_put++;
			}
		}
	}
	return n_seg_put;
}
//------------------------------------------------------------------------------------------------------
void CreaSegmenti() {

	Segmento sw; segmento.clear(); double s0;
	for (int i = 1; i<int(polyline.size()); i++) {
	    s0= 0.0;
		for (int k = 0; k<int(polyline[i].points.size())-1; k++) {
			sw.set(i, s0, polyline[i].points[k], polyline[i].points[k + 1]); 
			segmento.push_back(sw);
			s0+=sw.length;
		}
	}
	cout << "CreaSegmenti: ho creato " << segmento.size() << " Segmenti " << endl;

}
//----------------------------------------------------------------------------------------------------
void CreaMappa(void)
{
	static bool prima_volta = true;
	if (prima_volta) prima_volta = false;
	else { for (int j = 0; j<jmax; j++) delete[]A[j]; delete[]A; }

	CreaSegmenti();

	int n_seg_put = 0; int n_nodi_put = 0;
	c_ris1 = 1.72*RISOLUZIONE_MAPPA; c_ris2 = c_ris1*c_ris1;
	ds_lat = RISOLUZIONE_MAPPA / DSLAT;
	ds_lon = RISOLUZIONE_MAPPA / DSLON;
	jmax = int(1 + (LAT_MAX - LAT_MIN) / ds_lat);
	imax = int(1 + (LON_MAX - LON_MIN) / ds_lon);
	cout << "CreaMappa: VeneziaSM Mappa[" << imax << "," << jmax << "]";
	cout << " LX[km]= " << (LAT_MAX - LAT_MIN)*DSLAT / 1000;
	cout << " LY[km]= " << (LON_MAX - LON_MIN)*DSLON / 1000 << endl;

	A = new Elenco*[jmax]; for (int j = 0; j<jmax; j++) A[j] = new Elenco[imax];  // A[jmax][imax]
	for (int n = 0; n< int(segmento.size()); n++) n_seg_put += A_put_segmento(n);
	cout << "CreaMappa: ho inserito " << n_seg_put << " Segmenti " << endl;

	cout << "CreaMappa: n_nodi= " << n_nodi << endl;

	for (int n = 1; n< n_nodi; n++) n_nodi_put += A_put_nodo(n);
	cout << "CreaMappa: ho inserito " << n_nodi_put << " Nodi " << endl;

}

//----------------------------------------------------------------------------------------------------
int CercaPolyVicina(double lon, double lat, list <int> &vicini) {
	vicini.clear();
	int i = int((lon - LON_MIN) / ds_lon); int j = int((lat - LAT_MIN) / ds_lat);
	int n_vicini = int(A[j][i].segmento_id.size()); if (n_vicini == 0) return n_vicini;
	for (int k = 0; k< n_vicini; k++) vicini.push_back(segmento[A[j][i].segmento_id[k]].id_poly);
	vicini.sort(); vicini.unique();
	return int(vicini.size());
}
//-----------------------------------------------------------------------------------------------
bool CercaNodiVicini(double lon, double lat, list <NodoVicino> &nodoVicino) {
	nodoVicino.clear(); NodoVicino nv;
	int i = int((lon - LON_MIN) / ds_lon); int j = int((lat - LAT_MIN) / ds_lat);
	int n_vicini = int(A[j][i].nodo_id.size()); if (n_vicini == 0) return false;

	for (auto n : A[j][i].nodo_id) { nv.distanza= nodo[n].distanza(lon,lat); nv.id_nodo= n; nodoVicino.push_back(nv);}
	nodoVicino.sort(CompNodoVicino);
	if(nodoVicino.size() > 6) nodoVicino.resize(6);
	return true;
}
//----------------------------------------------------------------------------------------------------
bool CercaNodoVicino(double lon, double lat, double &distanza, int &id_nodo) {
	distanza= 1.0e8; id_nodo= 0;
	list <NodoVicino> nodoVicino; 
	int i = int((lon - LON_MIN) / ds_lon); int j = int((lat - LAT_MIN) / ds_lat);
	int n_vicini = int(A[j][i].nodo_id.size()); if (n_vicini == 0) return false;
	for (auto n : A[j][i].nodo_id) {
		double nodo_dist= nodo[n].distanza(lon,lat);
		if( distanza > nodo_dist ){ id_nodo= n; distanza = nodo_dist; }
	}
	if( id_nodo== 0) return false; else return true;
}
//----------------------------------------------------------------------------------------------------
int CercaPolyVicina(double x, double y, double &distanza, int &id_poly) {
	distanza = 1.0e10; id_poly = 0;
	int i = int((x - LON_MIN) / ds_lon);
	int j = int((y - LAT_MIN) / ds_lat);
	int n_vicini = int(A[j][i].segmento_id.size());
	if (n_vicini == 0) return n_vicini;
	distanza = segmento[A[j][i].segmento_id[0]].distanza(x, y); id_poly = segmento[A[j][i].segmento_id[0]].id_poly;
	for (int k = 1; k< n_vicini; k++) {
		double dd = segmento[A[j][i].segmento_id[k]].distanza(x, y);
		if (dd < distanza) { distanza = dd; id_poly = segmento[A[j][i].segmento_id[k]].id_poly; }
	}
	return n_vicini;
}
//----------------------------------------------------------------------------------------------------

