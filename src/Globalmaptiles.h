#pragma once

#include <common.h>
#include <set>

//GlobalMercator (based on EPSG:900913 = EPSG:3785)
//for Google Maps, Yahoo Maps, Microsoft Maps compatible tiles

struct GlobalMercator {
  int tile_size=256;
  double init_resolution, origin_shift;
  void init();
  pair<double, double> LatLonToMeters(double lat, double lon);
  pair <double, double> MetersToLatLon(double mx, double my);
  pair <double, double> PixelsToMeters(int px, int py, int zoom);
  pair<int, int> MetersToPixels(double mx, double my, int zoom);
  pair<int, int> PixelsToTile(double px, double py);
  pair<int, int> MetersToTile(double mx, double my, int zoom);
  pair <pair<double, double>, pair<double, double>> TileBounds(int tx, int ty, int zoom);
  pair <pair<double, double>, pair<double, double>> TileLatLonBounds( int tx, int ty, int zoom);
  pair <int, int> GoogleTile(int tx, int ty, int zoom);
  string QuadTree(int tx, int ty, int zoom);
  int tmin_x, tmin_y, tmax_x, tmax_y;

};

struct MapTile {
  pair <int, int> google_tile;
  string quadkey;
  pair< pair<double, double>, pair<double, double>> bounds; //<min<lat, lon>, max<lat, lon>>
  double lat_min, lat_max, lon_min, lon_max;
  vector<pair<int, int>> first_conn_cells;
  set<int> poly_in;
  int count = 0;
  vector<int> count_vec;
  bool best = false;
  bool cam = false;
};

void make_grid_map();
void bound_maptile(int zoom, double lat_min, double lat_max, double lon_min, double lon_max);
void bound_maptile_telecom(int zoom, double lat_min, double lat_max, double lon_min, double lon_max);
pair<int, int> LatLontoTile(double lat, double lon);
pair<int, int> LatLontoTimTile(double lat, double lon);
void poly_connection();
void poly_connection_tim();
void subnet_connection();

void cam_tiles();
void cam_timtiles();
void import_data(string file_name, map<string, int> &time_name);
void crea_allineatim(string file_name);
void leggi_allineatim(string file_name);
void import_data_tim(string file_name, map<string, int> &time_name);
void print_obstile_file();
void print_total_file();
void import_all_data();
void import_file();

void save_grid_heatmap(const string &imgname);
