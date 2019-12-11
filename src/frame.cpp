#include <common.h>
#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Slider.H>
#include "frame.h"
#include "GLtrans.h"
#include "mappa.h"
#include "polyline.h"
#include "Globalmaptiles.h"

extern bool mostraPoly, mostraNodo, reDraw, mostraGrid;
extern Fl_Output *linea1;
extern int screen_w, screen_h;
extern vector <PolyLine> polyline;
extern double alfa_zoom_cerchio;
extern void draw_init(void);
extern void draw_scene(void);
extern double lat0, lon0, dlat, dlon;
extern double alfa_zoom;
extern Fl_Value_Slider *zoom;
extern map<pair<int, int>, MapTile> grid_map;

list <int> path;
double delta_lon = 0.0, delta_lat = 0.0;
int poly_evidenziata = 0, nodo_evidenziato = 0;
MapTile cella_evidenziata;
GLdouble x_mouse, y_mouse, z_mouse;
int nodo_1, nodo_2;

//-------------------------------------------------------------------------------------------------
void Frame::draw() {

  if (!valid()) {
    glClearColor(0.0, 0.0, 0.0, 1);                        // Turn the background color black
    glViewport(0, 0, w(), h());                               // Make our viewport the whole window
    glMatrixMode(GL_PROJECTION);                           // Select The Projection Matrix
    glLoadIdentity();                                      // Reset The Projection Matrix
    gluOrtho2D(lon0 - dlon, lon0 + dlon, lat0 - dlat, lat0 + dlat);
    glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
    glLoadIdentity();                                      // Reset The Modelview Matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear The Screen And The Depth Buffer
    glLoadIdentity();                                      // Reset The View
    glEnable(GL_DEPTH_TEST);

    draw_init();
    valid(1);
  }
  draw_scene();
}
//-------------------------------------------------------------------------------------------------
int Frame::handle(int evento)
{
  int i_mouse, j_mouse, ierr;  //static int i_poly=-1;
  float iz;

  static char messaggio[300];

  switch (evento)
  {
  case FL_MOVE:
    reDraw = true;
    i_mouse = Fl::event_x();  j_mouse = screen_h - Fl::event_y(); iz = 0.;
    ierr = UnProject(GLdouble(i_mouse), GLdouble(j_mouse), GLdouble(iz), &x_mouse, &y_mouse, &z_mouse);
    sprintf(messaggio, " %10.6lf %10.6lf", y_mouse, x_mouse);
    glDisable(GL_DEPTH_TEST); linea1->value(messaggio); glEnable(GL_DEPTH_TEST);
    break;
  case FL_PUSH:
    reDraw = true;
    i_mouse = Fl::event_x(); j_mouse = screen_h - Fl::event_y(); iz = 0.;
    ierr = UnProject(GLdouble(i_mouse), GLdouble(j_mouse), GLdouble(iz), &x_mouse, &y_mouse, &z_mouse);

    if (Fl::event_button() == 3) {     //  1 il sinistro  2 entrambi 3 il destro
      delta_lon = -(x_mouse - lon0);
      delta_lat = -(y_mouse - lat0);
      break;
    }
    else if (Fl::event_button() == 1) {     //  1 il sinistro  2 entrambi 3 il destro
      double distanza_poly, distanza_nodo;
      if (mostraPoly)               CercaPolyVicina(x_mouse, y_mouse, distanza_poly, poly_evidenziata);
      if (mostraNodo)               CercaNodoVicino(x_mouse, y_mouse, distanza_nodo, nodo_evidenziato);
      std::cout << " distanza= " << distanza_poly << " poly_evidenziata= " << poly_evidenziata << std::endl;
      std::cout << " distanza= " << distanza_nodo << " nodo_evidenziato= " << nodo_evidenziato << std::endl;
      if (mostraGrid) {
        //pair<int, int> tile = LatLontoTile(y_mouse, x_mouse);
        pair<int, int> tile = LatLontoTimTile(y_mouse, x_mouse);
        cella_evidenziata = grid_map[make_pair(tile.first, tile.second)];
        cout << cella_evidenziata.lat_max << "\t" << cella_evidenziata.lon_max << endl;
      }
    }
    break;
  case FL_MOUSEWHEEL:
    alfa_zoom -= 0.125 * Fl::event_dy();
    if (alfa_zoom < zoom->minimum()) alfa_zoom = zoom->minimum();
    if (alfa_zoom > zoom->maximum()) alfa_zoom = zoom->maximum();
    zoom->value(sqrt(alfa_zoom));
    reDraw = true;
  default:
    break;
  }
  //  redraw();
  return 1;
}

