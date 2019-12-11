#include <common.h>
#include "polyline.h"
#include "leggi_dati.h"
#include <boost/algorithm/string.hpp>


vector <PolyLine> polyline; int n_poly = 0;
map <int, int> poly_cid2lid;
map<pair<int, int>, pair<int, int>> corr_tiles;

//------------------------------------------------------------------------------------------------------
void LeggiPoly(string city) {

  string poly_pnt_file = POLY_DIR + city + "_strade_pnt.txt";
  string poly_pro_file = POLY_DIR + city + "_strade_pro.txt";

	FILE *fp0; fp0 = fopen(poly_pnt_file.c_str(), "r");
	if (fp0 == NULL) { cout << "fopen error in " << poly_pnt_file << endl; exit(5); }

	int id, nPT, ilat, ilon;
	long long int cid;
	vector <PointXY> punti; PointXY punto_w; PolyLine polyline_w;
	bool finito = false;
	polyline.clear();
	//Inserisco una polyline vuota in posizione 0 (le polyline sono indicizzate [1:N]
	punti.clear();
	polyline_w.set(0, 0, punti); polyline.push_back(polyline_w); polyline_w.clear();

	//Inserisco i punti delle polylines
	while (fscanf(fp0, " %lld %d %d ", &cid, &id, &nPT) != EOF) {
		for (int i = 0; i < nPT; i++) {
			if (fscanf(fp0, " %d %d", &ilat, &ilon) == EOF) {
				cout << "errore di lettura in " << poly_pnt_file << endl; fflush(stdout); exit(5);
			}
			punto_w.set(ilon, ilat); punti.push_back(punto_w);
		}
		polyline_w.set(id, cid, punti);
		polyline.push_back(polyline_w);
		punti.clear(); polyline_w.clear();
	}

	fclose(fp0);

	cout << "leggiPoly: ho letto " << polyline.size() - 1 << " polyline da " << poly_pnt_file << endl;   fflush(stdout);
	n_poly = int(polyline.size());

	int n_pnt = 0; for (int i = 0; i < n_poly; i++) n_pnt += polyline[i].nPoints;
	cout << "leggiPoli:    n_pnt= " << n_pnt << " n_poly=" << n_poly << endl;


	// Leggi proprieta' strade

	fp0 = fopen(poly_pro_file.c_str(), "r");
	if (fp0 == NULL) { cout << "fopen error in " << poly_pro_file << endl; exit(5); }

	int frc, n2c, fow, oneway, kmh, lanes; char *name = new char[70];
	long long int id_strada, cid_Fjnct, cid_Tjnct;
	float meters;

	int n = 0;
	// FRONT TAIL VS INIZIO FINE
	while (fscanf(fp0, " %lld %lld %lld %f %d %d %d %d %d %d %s",
		&id_strada, &cid_Fjnct, &cid_Tjnct, &meters, &frc, &n2c, &fow, &oneway, &kmh, &lanes, name) != EOF) {
		n++;
		if (polyline[n].cid_poly == id_strada) {
			polyline[n].set(cid_Fjnct, cid_Tjnct, meters, frc, n2c, fow, oneway, kmh, lanes, name);
			polyline[n].CalcolaLength();
      poly_cid2lid[(int)id_strada] = n;
		}
		else { cout << " errore di indicizzazione nella polyline[" << n << "]  " << endl; exit(5); }
	}
	fclose(fp0);

	cout << "leggiPoly: ho letto " << n << " polyline data da " << poly_pro_file << endl;


	for (int i = 1; i<n_poly; i++)	polyline[i].CalcolaLength();

#if !defined JULY_15 && !defined JULY_16
  polyline.erase(polyline.begin() + poly_cid2lid[9243649]);
  --n_poly;
#endif // !defined (JULY_15) || !defined(JULY_16)

	//fflush(stdout); exit(4);
}
//------------------------------------------------------------------------------------------------------
void StampaPoly(string city) {

	FILE *fp0;
	string poly_pnt_out_file = "../output/" + city + "_strade_pnt.txt";
	string poly_pro_out_file = "../output/" + city + "_strade_pro.txt";

	fp0 = fopen(poly_pnt_out_file.c_str(), "w");
	if (fp0 == NULL) { cout << "fopen error in " << poly_pnt_out_file << endl; exit(5); }
	for (int i = 1; i < n_poly; i++) if (polyline[i].ok) polyline[i].stampa_pnt(fp0);
	fclose(fp0);

	fp0 = fopen(poly_pro_out_file.c_str(), "w");
	if (fp0 == NULL) { cout << "fopen error in " << poly_pro_out_file << endl; exit(5); }
	for (int i = 1; i < n_poly; i++) if (polyline[i].ok) polyline[i].stampa_pro(fp0);
	fclose(fp0);

}
//------------------------------------------------------------------------------------------------------
//temporarily added to show time correlated tiles
void ReadCorrTiles(string path_file) {
  FILE *fp0; fp0 = fopen(path_file.c_str(), "r");
  if (fp0 == NULL) { cout << "fopen error in " << path_file << endl; exit(5); }
  int tile_x, tile_y, tile_x_corr, tile_y_corr;
  while (fscanf(fp0, " %d,%d,%d,%d", &tile_x, &tile_y, &tile_x_corr, &tile_y_corr) != EOF) {
    corr_tiles[make_pair(tile_x, tile_y)] = make_pair(tile_x_corr, tile_y_corr);
  }
  cout << "size of corr_tiles: " << corr_tiles.size() << endl;
}
//------------------------------------------------------------------------------------------------------
