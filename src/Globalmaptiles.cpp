#include <common.h>
#include <cmath>
#include <string>
#include <boost/algorithm/string.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "Globalmaptiles.h"
#include "polyline.h"

using namespace cv;

map<pair<int, int>, MapTile> grid_map;
GlobalMercator mercator;
vector < vector<pair<int, int>>> tile_obs;
map<pair<int, int>, pair<int, int>> maptile_tim;
map<string, int> time15_name;

extern vector<PolyLine> polyline;
extern map<string, vector<int>> subnets;
extern string File;

void GlobalMercator::init() {
  init_resolution = 2 * PI * 6378137 / tile_size;
  //156543.03392804062 for tileSize 256 pixels
  origin_shift = 2 * PI * 6378137 / 2.0;
  //20037508.342789244
}
//-------------------------------------------------------------------------------------
pair<double, double> GlobalMercator::LatLonToMeters(double lat, double lon) {
  //Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913
  double mx = lon * origin_shift / 180.0;
  double my = log(tan((90 + lat) * PI / 360.0)) / (PI / 180.0);
  my = my * origin_shift / 180.0;
  return make_pair(mx, my);
}
//-------------------------------------------------------------------------------------
pair<double, double> GlobalMercator::MetersToLatLon(double mx, double my) {
  //Converts XY point from Spherical Mercator EPSG:900913 to lat/lon in WGS84 Datum
  double lon = (mx / origin_shift) * 180.0;
  double lat = (my / origin_shift) * 180.0;
  lat = 180 / PI * (2 * atan(exp(lat * PI / 180.0)) - PI / 2.0);
  return make_pair(lat, lon);
}
//-------------------------------------------------------------------------------------

pair<double, double> GlobalMercator::PixelsToMeters(int px, int py, int zoom) {
  //Converts pixel coordinates in given zoom level of pyramid to EPSG:900913

  double res = init_resolution / pow(2, zoom);
  double mx = px * res - origin_shift;
  double my = py * res - origin_shift;
  return make_pair(mx, my);
}

//-------------------------------------------------------------------------------------
pair<int, int> GlobalMercator::MetersToPixels(double mx, double my, int zoom) {
  //Converts EPSG:900913 to pyramid pixel coordinates in given zoom level"
  double res = init_resolution / pow(2, zoom);
  int py = int((my + origin_shift) / res);
  int px = int((mx + origin_shift) / res);
  return make_pair(px, py);
}
//-------------------------------------------------------------------------------------
pair<int, int> GlobalMercator::PixelsToTile(double px, double py) {
  //Returns a tile covering region in given pixel coordinates
  int tx = int(ceil(px / float(tile_size)) - 1);
  int ty = int(ceil(py / float(tile_size)) - 1);
  return make_pair(tx, ty);
}
//-------------------------------------------------------------------------------------
pair<int, int> GlobalMercator::MetersToTile(double mx, double my, int zoom) {
  //Returns tile for given mercator coordinates
  pair <int, int>  p, t;
  p = MetersToPixels(mx, my, zoom);
  t = PixelsToTile(p.first, p.second);
  return t;
}
//-------------------------------------------------------------------------------------
pair <pair<double, double>, pair<double, double>> GlobalMercator::TileBounds(int tx, int ty, int zoom) {
  //Returns bounds of the given tile in EPSG:900913 coordinates
  pair<double, double> min = PixelsToMeters(tx*tile_size, ty*tile_size, zoom);          //<minx, miny>
  pair<double, double> max = PixelsToMeters((tx + 1)*tile_size, (ty + 1)*tile_size, zoom);  //<maxx, maxy>
  return make_pair(min, max);
}
//-------------------------------------------------------------------------------------
pair <pair<double, double>, pair<double, double>> GlobalMercator::TileLatLonBounds(int tx, int ty, int zoom) {
  //Returns bounds of the given tile in latutude/longitude using WGS84 datum
  pair <pair<double, double>, pair<double, double>> bounds = TileBounds(tx, ty, zoom);
  pair<double, double> Min = MetersToLatLon(bounds.first.first, bounds.first.second);    //<Minx, Miny>
  pair<double, double> Max = MetersToLatLon(bounds.second.first, bounds.second.second);  //<Maxx, Maxy>
  return make_pair(Min, Max);
}
//-------------------------------------------------------------------------------------
pair <int, int> GlobalMercator::GoogleTile(int tx, int ty, int zoom) {
  //Converts TMS tile coordinates to Google Tile coordinates
  //coordinate origin is moved from bottom - left to top - left corner of the extent
  int temp = int(pow(2, zoom) - 1) - ty;
  return make_pair(tx, temp);
}
//-------------------------------------------------------------------------------------
string GlobalMercator::QuadTree(int tx, int ty, int zoom) {
  //Converts TMS tile coordinates to Microsoft QuadTree
  string quadKey;
  ty = int(pow(2, zoom) - 1) - ty;
  for (int i = zoom; i != 0; --i) {
    int digit = 0;
    int mask = 1 << (i - 1);
    if ((tx & mask) != 0)
      digit += 1;
    if ((ty & mask) != 0)
      digit += 2;
    quadKey.append(to_string(digit));
  }
  return quadKey;
}

void bound_maptile(int zoom, double lat_min, double lat_max, double lon_min, double lon_max) {
  ofstream out("../output/MapTile.csv");
  //map<pair<int, int>,MapTile> Grid_map;
  mercator.init();
  int tz = zoom;
  pair<double, double> m = mercator.LatLonToMeters(lat_min, lon_min);
  //cout << fixed << setprecision(4) << "Spherical Mercator (ESPG:900913) coordinates for lat/lon: " << m.first << "\t" << m.second << endl;
  pair<int, int> tmin = mercator.MetersToTile(m.first, m.second, tz);
  m = mercator.LatLonToMeters(lat_max, lon_max);
  //cout << fixed << setprecision(4) << "Spherical Mercator (ESPG:900913) cooridnate for maxlat/maxlon: " << m.first << "\t" << m.second << endl;
  pair<int, int> tmax = mercator.MetersToTile(m.first, m.second, tz);

  mercator.tmin_x = tmin.first;
  mercator.tmin_y = int((pow(2, zoom) - 1) - tmax.second);
  mercator.tmax_x = tmax.first;
  mercator.tmax_y = int((pow(2, zoom) - 1) - tmin.second);
  //cout << "tmin_x  " << mercator.tmin_x << "tmax_x" << mercator.tmax_x << endl;
  //cout << "tmin_y  " << mercator.tmin_y << "tmax_y" << mercator.tmax_y << endl;

  for (int ty = tmin.second; ty <= tmax.second + 1; ++ty) {
    for (int tx = tmin.first; tx <= tmax.first + 1; ++tx) {
      out << tz << "/" << tx << "/" << ty << "( TileMapService: z / x / y )" << endl;
      pair<int, int> g = mercator.GoogleTile(tx, ty, tz);
      out << "\tGoogle: " << g.first << "\t" << g.second << endl;
      string quadkey = mercator.QuadTree(tx, ty, tz);
      out << "\tQuadkey: " << quadkey << endl;
      pair<pair<double, double>, pair<double, double>> bounds = mercator.TileBounds(tx, ty, tz);
      out << fixed << setprecision(5) << "\tEPSG:900913 Extent: " << bounds.first.first << "\t" << bounds.first.second << "\t" << bounds.second.first << "\t" << bounds.second.second << endl;
      pair<pair<double, double>, pair<double, double>> wgsbounds = mercator.TileLatLonBounds(tx, ty, tz);
      out << "\tWGS84 Extent: " << fixed << setprecision(6) << wgsbounds.first.first << "\t" << wgsbounds.first.second << "\t" << wgsbounds.second.first << "\t" << wgsbounds.second.second << endl;
      MapTile mp;
      mp.bounds = bounds;
      mp.lat_min = wgsbounds.first.first;
      mp.lon_min = wgsbounds.first.second;
      mp.lat_max = wgsbounds.second.first;
      mp.lon_max = wgsbounds.second.second;

      // mp.lat_max = wgsbounds.first.first+0.0012500001;
      // mp.lon_max = wgsbounds.first.second+ 0.0016666668;

      mp.google_tile = g;
      mp.quadkey = quadkey;
      grid_map[g] = mp;
    }
  }

}
//------------------------------------------------------------------------------------------------
double LAT_min;
double LON_min;
double dlat_tim = 0.0012500001;
double dlon_tim = 0.0016666668;
int Nrows, Ncols;
void bound_maptile_telecom(int zoom, double lat_min, double lat_max, double lon_min, double lon_max) {
  //map<pair<int, int>,MapTile> Grid_map;
  mercator.init();
  int tz = zoom;
  pair<double, double> m = mercator.LatLonToMeters(lat_min, lon_min);
  //cout << fixed << setprecision(4) << "Spherical Mercator (ESPG:900913) coordinates for lat/lon: " << m.first << "\t" << m.second << endl;
  pair<int, int> tmin = mercator.MetersToTile(m.first, m.second, tz);
  m = mercator.LatLonToMeters(lat_max, lon_max);
  //cout << fixed << setprecision(4) << "Spherical Mercator (ESPG:900913) cooridnate for maxlat/maxlon: " << m.first << "\t" << m.second << endl;
  pair<int, int> tmax = mercator.MetersToTile(m.first, m.second, tz);

  mercator.tmin_x = tmin.first;
  mercator.tmin_y = int((pow(2, zoom) - 1) - tmax.second);
  mercator.tmax_x = tmax.first;
  mercator.tmax_y = int((pow(2, zoom) - 1) - tmin.second);

  int ty0 = tmin.second;
  int tx0 = tmin.first;
  pair<pair<double, double>, pair<double, double>> wgsbounds0 = mercator.TileLatLonBounds(tx0, ty0, tz);
  MapTile mp0;
  mp0.lat_min = wgsbounds0.first.first;
  mp0.lon_min = wgsbounds0.first.second;

  mp0.lat_max = wgsbounds0.first.first + 0.0012500001;
  mp0.lon_max = wgsbounds0.first.second + 0.0016666668;
  Nrows = int(((LAT_MAX - mp0.lat_min) / 0.0012500001));
  Ncols = int(((LON_MAX - mp0.lon_min) / 0.0016666668));
  cout << "Nrows " << Nrows << "  Ncols " << Ncols << endl;
  LAT_min = wgsbounds0.first.first;
  LON_min = wgsbounds0.first.second;

  for (int ty = 0; ty <= Nrows; ++ty) {
    for (int tx = 0; tx <= Ncols; ++tx) {
      pair<int, int> g = make_pair(tx, Nrows - ty);
      MapTile mp;
      mp.lat_min = LAT_min + ty * dlat_tim;
      mp.lon_min = LON_min + tx * dlon_tim;
      mp.lat_max = LAT_min + (ty + 1) * dlat_tim;
      mp.lon_max = LON_min + (tx + 1) * dlon_tim;
      mp.google_tile = g;
      grid_map[g] = mp;
    }
  }

}
//------------------------------------------------------------------------------------------------
pair<int, int> LatLontoTile(double lat, double lon) {
  mercator.init();
  pair<double, double> meters = mercator.LatLonToMeters(lat, lon);
  pair<int, int> tiles = mercator.MetersToTile(meters.first, meters.second, ZOOM_LEVEL);
  pair<int, int> goog_cord = mercator.GoogleTile(tiles.first, tiles.second, ZOOM_LEVEL);
  return goog_cord;
}
//------------------------------------------------------------------------------------------------
pair<int, int> LatLontoTimTile(double lat, double lon) {

  int i = int((lon - LON_min) / dlon_tim);
  int j = int(Nrows + 1 - (lat - LAT_min) / dlat_tim);
  pair<int, int> tim_tile = make_pair(i, j);
  return tim_tile;
}
//----------------------------------------------------------------------------------------
bool find_corrisp(MapTile tile1, MapTile tile2) {
  bool ok = false;
  for (auto i : tile1.poly_in) {
    for (auto j : tile2.poly_in) {
      if (i == j) {
        ok = true;
        break;
      }
    }
    if (ok) break;
  }
  return ok;
}
//------------------------------------------------------------------------------------------------
void average(pair<double, double> cord1, pair<double, double> cord2, vector<pair<double, double>> &coll_av) {
  double dx = DSLON * (cord2.second - cord1.second);
  double dy = DSLAT * (cord2.first - cord1.first);
  double dist = sqrt(dx * dx + dy * dy);
  if (dist < 50.0) return;
  else {
    double lon_av = abs(cord2.second + cord1.second) / 2.;
    double lat_av = abs(cord2.first + cord1.first) / 2.;
    pair<double, double> av = make_pair(lat_av, lon_av);
    coll_av.push_back(av);
    average(cord1, av, coll_av);
    average(av, cord2, coll_av);
  }

}
//------------------------------------------------------------------------------------------------
void poly_connection() {
  int count = 0;
  for (auto n : polyline) {
    for (int m = 0; m < n.points.size(); ++m) {
      //cout << "punto: " << m << endl;
      pair<int, int> tile = LatLontoTile(n.points[m].lat, n.points[m].lon);
      grid_map[tile].poly_in.insert(n.id_local);
      //seconda versione
      if (m < n.points.size() - 1) {
        vector<pair<double, double>> added_points;
        pair< double, double> cord1 = make_pair(n.points[m].lat, n.points[m].lon);
        pair< double, double> cord2 = make_pair(n.points[m + 1].lat, n.points[m + 1].lon);
        average(cord1, cord2, added_points);
        for (const auto i : added_points) {
          pair<int, int> tile = LatLontoTile(i.first, i.second);
          grid_map[tile].poly_in.insert(n.id_local);
        }
      }
    }
  }

  //maschera dei conteggi
  for (int i = mercator.tmin_x + 1; i < mercator.tmax_x; ++i) {
    for (int j = mercator.tmin_y + 1; j < mercator.tmax_y; ++j) {
      for (int it = (i - 1); it <= (i + 1); ++it) {
        if (find_corrisp(grid_map[make_pair(it, j - 1)], grid_map[make_pair(i, j)]))
          grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(it, j - 1));
      }
      for (int it = i - 1; it <= i + 1; ++it) {
        if (find_corrisp(grid_map[make_pair(it, j + 1)], grid_map[make_pair(i, j)]))
          grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(it, j + 1));
      }
      if (find_corrisp(grid_map[make_pair(i - 1, j)], grid_map[make_pair(i, j)]))
        grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(i - 1, j));
      if (find_corrisp(grid_map[make_pair(i + 1, j)], grid_map[make_pair(i, j)]))
        grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(i + 1, j));
    }
  }


}
//------------------------------------------------------------------------------------------------
void poly_connection_tim() {
  int count = 0;
  for (auto n : polyline) {
    for (int m = 0; m < n.points.size(); ++m) {
      pair<int, int> tile = LatLontoTimTile(n.points[m].lat, n.points[m].lon);
      grid_map[tile].poly_in.insert(n.id_local);
      if (m < n.points.size() - 1) {
        vector<pair<double, double>> added_points;
        pair< double, double> cord1 = make_pair(n.points[m].lat, n.points[m].lon);
        pair< double, double> cord2 = make_pair(n.points[m + 1].lat, n.points[m + 1].lon);
        average(cord1, cord2, added_points);
        for (const auto i : added_points) {
          pair<int, int> tile = LatLontoTimTile(i.first, i.second);
          grid_map[tile].poly_in.insert(n.id_local);
        }
      }
    }
  }

  //maschera dei conteggi
  for (int i = 1; i < Ncols; ++i) {
    for (int j = 1; j < Nrows; ++j) {
      for (int it = (i - 1); it <= (i + 1); ++it) {
        if (find_corrisp(grid_map[make_pair(it, j - 1)], grid_map[make_pair(i, j)]))
          grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(it, j - 1));
      }
      for (int it = i - 1; it <= i + 1; ++it) {
        if (find_corrisp(grid_map[make_pair(it, j + 1)], grid_map[make_pair(i, j)]))
          grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(it, j + 1));
      }
      if (find_corrisp(grid_map[make_pair(i - 1, j)], grid_map[make_pair(i, j)]))
        grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(i - 1, j));
      if (find_corrisp(grid_map[make_pair(i + 1, j)], grid_map[make_pair(i, j)]))
        grid_map[make_pair(i, j)].first_conn_cells.push_back(make_pair(i + 1, j));
    }
  }
}
//------------------------------------------------------------------------------------------------
vector<string> name_cam;
void cam_tiles() {
  vector<pair<double, double>> cam_tile;
  cam_tile = {
    make_pair(45.437904, 12.321152), //Papadopoli
    make_pair(45.438229, 12.335624), //Delvin
    make_pair(45.433178, 12.330780), //Spezier
    make_pair(45.438962, 12.337114), //Grisostomo
    make_pair(45.441388, 12.323173), //Stazione (Aggiunta)
    make_pair(45.443421,12.331347), //StradaNuova
    make_pair(45.434650, 12.336776) //San Marco
  };
  name_cam.push_back("Papadopoli");
  name_cam.push_back("Delvin");
  name_cam.push_back("Spezier");
  name_cam.push_back("Grisostomo");
  name_cam.push_back("Stazione");
  name_cam.push_back("StradaNuova");
  name_cam.push_back("SanMarco");

  int cnt = 0;
  for (auto &s : cam_tile) {
    vector<pair<int, int>> tile_obs_1;
    pair<int, int> _tile = LatLontoTile(s.first, s.second);
    grid_map[_tile].cam = true;
    tile_obs_1.push_back(_tile);
    cnt++;
    for (auto &n : grid_map[_tile].first_conn_cells)
    {
      grid_map[n].cam = true;
      tile_obs_1.push_back(n);
      cnt++;
    }
    tile_obs.push_back(tile_obs_1);
  }
  cout << "Celle di osservazione: " << cnt << endl;
}
//------------------------------------------------------------------------------------------------
void cam_timtiles() {
  vector<pair<double, double>> cam_tile;
  cam_tile = {
    make_pair(45.437904, 12.321152), //Papadopoli
    make_pair(45.438229, 12.335624), //Delvin
    make_pair(45.433178, 12.330780), //Spezier
    make_pair(45.438962, 12.337114), //Grisostomo
    make_pair(45.441388, 12.323173), //Stazione (Aggiunta)
    make_pair(45.443421,12.331347), //StradaNuova (Aggiunta)
    make_pair(45.434650, 12.336776) //San Marco
  };
  name_cam.push_back("Papadopoli");
  name_cam.push_back("Delvin");
  name_cam.push_back("Spezier");
  name_cam.push_back("Grisostomo");
  name_cam.push_back("Stazione");
  name_cam.push_back("StradaNuova");
  name_cam.push_back("SanMarco");

  int cnt = 0;
  for (auto &s : cam_tile) {
    vector<pair<int, int>> tile_obs_1;
    pair<int, int> _tile = LatLontoTimTile(s.first, s.second);
    grid_map[_tile].cam = true;
    tile_obs_1.push_back(_tile);
    cnt++;
    for (auto &n : grid_map[_tile].first_conn_cells)
    {
      grid_map[n].cam = true;
      tile_obs_1.push_back(n);
      cnt++;
    }
    tile_obs.push_back(tile_obs_1);
  }
  cout << "Celle di osservazione: " << cnt << endl;
  ofstream out("../output/analisi_griglia/tile_conversion.txt");
  for (int i = 0; i < (int)tile_obs.size(); ++i) {
    for (const auto &p : tile_obs[i]) {
      auto mtile = std::find_if(maptile_tim.begin(), maptile_tim.end(), [p](const pair<pair<int, int>, pair<int, int>> &pair)
      {
        return pair.second.first == p.first && pair.second.second == p.second;
      });

      out << name_cam[i] << "\t" << mtile->first.first << "\t" << mtile->first.second << endl;

    }

  }

}
//------------------------------------------------------------------------------------------------
int Ifile = 0;
void import_data(string file_name, map<string, int> &time_name) {
  ifstream input(file_name);
  if (!input) cout << "ERROR: unable to read input file " << endl;
  string day;
  day = file_name.substr(file_name.size() - 10, 6);
  string linea;
  vector<string> strs;
  getline(input, linea); // salto una riga
  boost::split(strs, linea, boost::is_any_of(","), boost::token_compress_off);
  for (int i = 2; i < strs.size(); ++i) {
    string temp = day + "_" + strs[i];
    time_name[temp] = (96 * Ifile) + i - 2;
  }

  while (input) {
    linea.clear();
    strs.clear();
    getline(input, linea);
    boost::split(strs, linea, boost::is_any_of(","));
    if (strs.size() > 1) {
      pair<int, int> tile = make_pair(stoi(strs[0]), stoi(strs[1]));
      if (tile.first<mercator.tmax_x && tile.first>mercator.tmin_x && tile.second<mercator.tmax_y && tile.second>mercator.tmin_y){
        for (int i = 2; i < strs.size(); ++i){
          if (strs[i] == "") {
            grid_map[tile].count_vec.push_back(0);
          }
          else {
            grid_map[tile].count_vec.push_back(stoi(strs[i]));
          }
        }
      }
    }
  }
  input.close();
  Ifile++;
}
//-------------------------------------------------------------------------------------
void crea_allineatim(string file_name) {
  ifstream input(file_name);
  if (!input) cout << "ERROR: unable to read input file " << endl;
  string linea;
  vector<string> strs;
  map<int, int> TiletoTim_x;
  map<int, int> TiletoTim_y;
  getline(input, linea); // salto una riga
  while (input) {
    linea.clear();
    strs.clear();
    getline(input, linea);
    boost::split(strs, linea, boost::is_any_of(","));
    if (strs.size() > 1) {
      pair<int, int> tile = make_pair(stoi(strs[0]), stoi(strs[1]));
      if (tile.first <= mercator.tmax_x && tile.first >= mercator.tmin_x && tile.second <= mercator.tmax_y && tile.second >= mercator.tmin_y)
      {
        TiletoTim_x[tile.first] = 0;
        TiletoTim_y[tile.second] = 0;
      }
    }
  }
  input.close();
  int cnt_i = 0;
  int cnt_j = 0;
  for (auto &i : TiletoTim_x)
  {
    i.second = cnt_i;
    cnt_i++;
  }
  for (auto &j : TiletoTim_y)
  {
    j.second = cnt_j;
    cnt_j++;
  }
  cout << "Ci sono: " << cnt_i - 1 << " tile x" << endl;
  cout << "Ci sono: " << cnt_j - 1 << " tile y" << endl;
  ofstream outp("../input/DATI/olivetti/maptiletim.csv");
  for (auto i : TiletoTim_x) {
    for (auto j : TiletoTim_y) {
      outp << i.first << "\t" << j.first << "\t" << i.second << "\t" << j.second << endl;
    }
  }


}
//-------------------------------------------------------------------------------------
void leggi_allineatim(string file_name) {
  ifstream input(file_name);
  if (!input) cout << "ERROR: unable to read input file " << endl;
  string linea;
  vector<string> strs;
  while (input) {
    linea.clear();
    strs.clear();
    getline(input, linea);
    boost::split(strs, linea, boost::is_any_of("\t"));
    if (strs.size() > 1) {
      maptile_tim[make_pair(stoi(strs[0]), stoi(strs[1]))] = make_pair(stoi(strs[2]), stoi(strs[3]));
    }
  }
  input.close();

}
//-------------------------------------------------------------------------------------
void import_data_tim(string file_name, map<string, int> &time_name) {
  ifstream input(file_name);
  if (!input) cout << "ERROR: unable to read input file " << endl;
  string day;
  day = file_name.substr(file_name.size() - 10, 6);
  string linea;
  vector<string> strs;
  getline(input, linea); // salto una riga
  boost::split(strs, linea, boost::is_any_of(","), boost::token_compress_off);
  for (int i = 2; i < strs.size(); ++i) {
    string temp = day + "_" + strs[i];
    time_name[temp] = (96 * Ifile) + i - 2;
  }

  cout << "day: " << day << endl;

  while (input) {
    linea.clear();
    strs.clear();
    getline(input, linea);
    boost::split(strs, linea, boost::is_any_of(","));
    if (strs.size() > 1) {
      pair<int, int> tile = make_pair(stoi(strs[0]), stoi(strs[1]));
      if (tile.first <= mercator.tmax_x && tile.first >= mercator.tmin_x && tile.second <= mercator.tmax_y && tile.second >= mercator.tmin_y) {
        for (int i = 2; i < strs.size(); ++i) {
          if (strs[i] == "") {
            grid_map[maptile_tim[tile]].count_vec.push_back(0);
          }
          else {
            grid_map[maptile_tim[tile]].count_vec.push_back(stoi(strs[i]));
          }
        }
      }
    }
  }
  input.close();
  Ifile++;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void print_obstile_file() {
  ofstream out("../output/analisi_griglia/obs_tiles.csv");
  out << "time,";
  for (auto n : name_cam)
    out << n << ",";
  out << endl;
  for (auto t : time15_name) {
    out << t.first << ",";
    for (int n = 0; n < name_cam.size(); ++n) {
      int cnt = 0;
      int total = 0;
      for (const auto &m : tile_obs[n]) {
        if (grid_map[m].count_vec.size() != 0) {
          total += grid_map[m].count_vec[t.second];
          cnt++;
        }
      }
      //total /= cnt;
      out << int(total) << ",";
    }
    out << endl;
  }
}
//-------------------------------------------------------------------------------------
void print_total_file() {
  ofstream total_file("../output/analisi_griglia/total.csv");
  for (auto t : time15_name) {
    int totale = 0;
    for (auto m : grid_map) {
      if (m.second.count_vec.size() != 0) {
        totale += m.second.count_vec[t.second];
      }
    }
    total_file << t.first << "\t" << totale << endl;
  }
}
//-------------------------------------------------------------------------------------
void import_all_data() {
  for (int i = 180514; i != 180515; ++i) {
    string file_name = "../input/DATI/olivetti/" + to_string(i) + ".csv";
    import_data_tim(file_name, time15_name);
  }
}
//-------------------------------------------------------------------------------------
void import_file() {
  ifstream input(File);
  if (!input) cout << "ERROR: unable to read input file " << endl;
  string day;
  day = File.substr(File.size() - 10, 6);
  string linea;
  vector<string> strs;
  getline(input, linea); // salto una riga
  boost::split(strs, linea, boost::is_any_of(","), boost::token_compress_off);
  cout << "day: " << day << endl;

  while (input) {
    linea.clear();
    strs.clear();
    getline(input, linea);
    boost::split(strs, linea, boost::is_any_of(","));
    if (strs.size() > 1) {
      pair<int, int> tile = make_pair(stoi(strs[0]), stoi(strs[1]));
      if (tile.first <= mercator.tmax_x && tile.first >= mercator.tmin_x && tile.second <= mercator.tmax_y && tile.second >= mercator.tmin_y) {
        if (strs[2] == "") {
          grid_map[maptile_tim[tile]].count_vec.push_back(0);
        }
        else {
          grid_map[maptile_tim[tile]].count_vec.push_back(stoi(strs[2]));
        }
      }
    }
  }
  input.close();

}

//-------------------------------------------------------------------------------------
void make_grid_map() {
  bound_maptile_telecom(ZOOM_LEVEL, LAT_MIN, LAT_MAX, LON_MIN, LON_MAX);
  poly_connection_tim();
  leggi_allineatim("../input/DATI/olivetti/maptiletim.csv");
  cam_timtiles();
  import_file();
  //import_alldata();
  //print_obstile_file();
  //print_total_file();
}
//-------------------------------------------------------------------------------------
void save_grid_heatmap(const string &imgname) {
  int r, c, cnt;
  auto col = Scalar(0, 0, 0);
  int scala = 40;
  Mat image = imread("../input/roi_standard.png", CV_LOAD_IMAGE_COLOR);
  //int wext = 1920;
  //int hext = 1080;
  //Mat image(Size(wext, hext), CV_8UC4, Scalar(255, 255, 255, 255));
  int wext = image.cols;
  int hext = image.rows;
  int dw = wext / (Ncols+1);
  int dh = hext / (Nrows+1);
  resize(image, image, Size((Ncols + 1)*dw, (Nrows + 1)*dh), 0, 0, CV_INTER_LINEAR);
  double alpha = 0.3;
  for (const auto &p : maptile_tim) {
    r = p.second.second;
    c = p.second.first;
    cnt = (grid_map[p.second].count_vec.size()) ? grid_map[p.second].count_vec.front() : 0;

    if (cnt < 6)              col = Scalar(230, 230, 255);
    else if (cnt < scala)     col = Scalar(205, 205, 255);
    else if (cnt < 3 * scala) col = Scalar(180, 180, 255);
    else if (cnt < 5 * scala) col = Scalar(130, 130, 255);
    else if (cnt < 7 * scala) col = Scalar(80, 80, 255);
    else                      col = Scalar(30, 30, 255);

    Mat roi = image(Rect(c*dw, r*dh, dw, dh));
    Mat color(roi.size(), CV_8UC3, col);
    addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
  }
  imwrite("test.png", image);
}

