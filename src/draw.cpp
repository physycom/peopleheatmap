#include <common.h>

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Output.H>
#include "polyline.h"
#include "nodo.h"
#include "draw.h"
#include "mappa.h"
#include "GLtrans.h"
#include "Globalmaptiles.h"

#define QUADRATO  1
#define SMALL_QUADRATO  2
#define SS_QUADRATO  3
#define DISCO 4

extern vector <PolyLine> polyline;  extern int n_poly;
extern vector <Nodo> nodo;          extern int n_nodi;
extern map<pair<int, int>, MapTile> grid_map;
extern map<string, int> time15_name;

extern bool mostraPoly, mostraNodo, mostraGrid, reDraw;
extern bool mostraSubnet;

extern int    graphicInView, timeInView;
extern double alfa_zoom, delta_lon, delta_lat, lat0, lon0, dlon, dlat, tempo_value;
extern char   titolo[100];

extern Fl_Output *linea1, *linea2;
extern int poly_evidenziata, nodo_evidenziato;
extern MapTile cella_evidenziata;
extern int screen_width, screen_height;
string name;
extern int nodo_1, nodo_2;
extern int w_est, h_est;

// subnet
extern map<string, vector<int>> subnets;
extern string subnet_select;

// ********************************************************************************************************
void draw_init(void) {
  gl_font(FL_HELVETICA_BOLD, 26);
  double LX = 0.006*dlon / alfa_zoom, LY = 0.7*LX;

  glNewList(QUADRATO, GL_COMPILE);
  glBegin(GL_QUADS);
  glVertex3d(-LX, -LY, 0.1); glVertex3d(LX, -LY, 0.1);
  glVertex3d(LX, LY, 0.1); glVertex3d(-LX, LY, 0.1);
  glEnd();
  glEndList();

  LX = 0.001*dlon / alfa_zoom; LY = 0.7*LX;
  glNewList(SMALL_QUADRATO, GL_COMPILE);
  glBegin(GL_QUADS);
  glVertex3d(-LX, -LY, 0.1); glVertex3d(LX, -LY, 0.1);
  glVertex3d(LX, LY, 0.1); glVertex3d(-LX, LY, 0.1);
  glEnd();
  glEndList();

  LX = 0.0006*dlon / alfa_zoom; LY = 0.7*LX;
  glNewList(SS_QUADRATO, GL_COMPILE);
  glBegin(GL_QUADS);
  glVertex3d(-LX, -LY, 0.1); glVertex3d(LX, -LY, 0.1);
  glVertex3d(LX, LY, 0.1); glVertex3d(-LX, LY, 0.1);
  glEnd();
  glEndList();

  glNewList(DISCO, GL_COMPILE);
  GLUquadricObj *disk; disk = gluNewQuadric();
  glScaled(1.0, 0.7, 1.0);
  gluDisk(disk, 0, 2 * LX, 10, 1); // gluDisk(disk, inDiameter, outDiameter, vertSlices, horizSlices);
  glEndList();

}
// ********************************************************************************************************
void draw_polyline() {
  glPushMatrix();
  for (int i = 1; i<int(polyline.size()); i++) {
    if (polyline[i].n2c == 6) glColor3d(0.4, 0.7, 0.7);
    else                      glColor3d(0.4, 0.4, 0.4);
    glBegin(GL_LINE_STRIP);
    for (int j = 0; j < polyline[i].nPoints; j++)
      glVertex3d(polyline[i].points[j].lon, polyline[i].points[j].lat, 0.05);
    glEnd();
  }
  glPopMatrix();
  glColor3d(1.0, 1.0, 1.0);
}
// ********************************************************************************************************
void draw_nodo_evidenziato() {

  int i = nodo_evidenziato; if (i <1 || i > n_nodi - 1) return;
  glColor3d(1.0, 0.0, 0.0);
  glPushMatrix(); 	glTranslated(nodo[i].lon, nodo[i].lat, 0.2); glCallList(QUADRATO); glPopMatrix();
  glColor3d(1.0, 1.0, 1.0);

  glDisable(GL_DEPTH_TEST);
  string s2 = " nodo_cid= " + to_string(nodo[i].cid_nodo) + " nodo_id= " + to_string(nodo[i].id_nodo);
  linea2->value(s2.c_str());
  glEnable(GL_DEPTH_TEST);

}
// ********************************************************************************************************
void draw_poly_evidenziata() {
  int i = poly_evidenziata;
  if (i <1 || i > n_poly - 1) return;

  glColor3d(1.0, 1.0, 1.0);  glLineWidth(2);
  glPushMatrix();
  glBegin(GL_LINE_STRIP);
  for (int j = 0; j < polyline[i].nPoints; j++)
    glVertex3d(polyline[i].points[j].lon, polyline[i].points[j].lat, 1.0);
  glEnd();
  glPopMatrix();
  glLineWidth(1);

  glDisable(GL_DEPTH_TEST);
  string s2 = to_string(i) + "   " + polyline[i].name + " poly_cid= " + to_string(polyline[i].cid_poly) + " length: " + to_string(polyline[i].length);
  linea2->value(s2.c_str());
  glEnable(GL_DEPTH_TEST);

}
// ********************************************************************************************************
void draw_grid() {
  //draw the grid ok 150x150 meters
  glColor3d(0.3, 0.3, 0.3);
  glPushMatrix();
  for (auto i : grid_map) {
    glBegin(GL_LINE_STRIP);
    glVertex3d(i.second.lon_min, i.second.lat_min, 0.1);
    glVertex3d(i.second.lon_min, i.second.lat_max, 0.1);
    glVertex3d(i.second.lon_max, i.second.lat_max, 0.1);
    glVertex3d(i.second.lon_max, i.second.lat_min, 0.1);
    glVertex3d(i.second.lon_min, i.second.lat_min, 0.1);
    glEnd();
  }
  glPopMatrix();

  glPushMatrix();
  int scala = 40;
  for (auto i : grid_map) {
    if (i.second.count_vec.size() != 0) {
      if (i.second.count_vec[0] < 6) glColor3d(1.0, 0.9, 0.9);
      else if (i.second.count_vec[0] < scala) glColor3d(1.0, 0.8, 0.8);
      else if (i.second.count_vec[0] < 3 * scala) glColor3d(1.0, 0.7, 0.7);
      else if (i.second.count_vec[0] < 5 * scala) glColor3d(1.0, 0.5, 0.5);
      else if (i.second.count_vec[0] < 7 * scala) glColor3d(1.0, 0.3, 0.3);
      else glColor3d(1.0, 0.1, 0.1);

      glBegin(GL_QUADS);
      glVertex3d(i.second.lon_min, i.second.lat_min, 0.0);
      glVertex3d(i.second.lon_min, i.second.lat_max, 0.0);
      glVertex3d(i.second.lon_max, i.second.lat_max, 0.0);
      glVertex3d(i.second.lon_max, i.second.lat_min, 0.0);
      glVertex3d(i.second.lon_min, i.second.lat_min, 0.0);
      glEnd();
    }
  }
  glPopMatrix();
  glColor3d(1.0, 1.0, 1.0);
}
// ********************************************************************************************************
void draw_cella_evidenziata() {

  glPushMatrix();
  glColor3d(1.0, 0.0, 0.0);
  glBegin(GL_LINE_STRIP);
  glVertex3d(cella_evidenziata.lon_min, cella_evidenziata.lat_min, 0.7);
  glVertex3d(cella_evidenziata.lon_min, cella_evidenziata.lat_max, 0.7);
  glVertex3d(cella_evidenziata.lon_max, cella_evidenziata.lat_max, 0.7);
  glVertex3d(cella_evidenziata.lon_max, cella_evidenziata.lat_min, 0.7);
  glVertex3d(cella_evidenziata.lon_min, cella_evidenziata.lat_min, 0.7);
  glEnd();
  glPopMatrix();

  glPushMatrix();
  for (auto n : cella_evidenziata.first_conn_cells) {
    glColor3d(1.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    glVertex3d(grid_map[n].lon_min, grid_map[n].lat_min, 0.65);
    glVertex3d(grid_map[n].lon_min, grid_map[n].lat_max, 0.65);
    glVertex3d(grid_map[n].lon_max, grid_map[n].lat_max, 0.65);
    glVertex3d(grid_map[n].lon_max, grid_map[n].lat_min, 0.65);
    glVertex3d(grid_map[n].lon_min, grid_map[n].lat_min, 0.65);
    glEnd();
  }
  glPopMatrix();
  glDisable(GL_DEPTH_TEST);
  string s2 = to_string(cella_evidenziata.google_tile.first) + "   " + to_string(cella_evidenziata.google_tile.second);
  linea2->value(s2.c_str());
  glEnable(GL_DEPTH_TEST);

}
// ********************************************************************************************************
void draw_scene() {
  reDraw = false;
  static bool primavolta = true;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glTranslated(lon0, lat0, 0.0);
  glScaled(alfa_zoom, alfa_zoom, 1.0);
  glTranslated(-lon0, -lat0, 0.0);
  glTranslated(delta_lon, delta_lat, 0.0);
  SaveModel();
  glPushMatrix();
  draw_polyline();
  if (mostraNodo) draw_nodo_evidenziato();
  if (mostraPoly) draw_poly_evidenziata();
  if (mostraGrid) draw_cella_evidenziata();
  if (mostraGrid) draw_grid();
  glPopMatrix();
  glPopMatrix();
}
// ********************************************************************************************************

// General purpose ****************************************************************************************
void color_palette(const int &i)
{
  switch (i % 4)
  {
  case 0:  glColor3d(198.f / 255.f, 44.f / 255.f, 1.f / 255.f); break; // red
  case 1:  glColor3d(198.f / 255.f, 194.f / 255.f, 1.f / 255.f); break; // lime
  case 2:  glColor3d(172.f / 255.f, 44.f / 255.f, 247.f / 255.f); break; // purple
  case 3:  glColor3d(255.f / 255.f, 249.f / 255.f, 81.f / 255.f); break; // yellow
  default: glColor3d(255.f / 255.f, 255.f / 255.f, 255.f / 255.f); break; // white
  }
}
