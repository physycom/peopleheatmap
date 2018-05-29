#include <cstdio>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/gl.h>
#include <FL/glu.h>
#include <metodi.h>
#include <polyline.h>

using namespace std;

// bella roba l'extern
int screen_w, screen_h, space, b_w, b_h;
bool mostraSubnet;
Fl_Check_Button *button32;
void set_off() {};
int subnet_select;
bool reDraw;
// fine bella roba

extern vector<PolyLine> polyline;
extern double lat0, lon0, dlat, dlon, zoom_start;

Fl_Gl_Window *scene;
Fl_Window *form;
double alfa_zoom;
double delta_lon, delta_lat;

void disegna_prova()
{
  glPushMatrix();
  glColor3d(0.4, 0.4, 0.4);
  glBegin(GL_LINE_STRIP);
  glVertex3d(lon0, lat0 + 1, 0.0);
  glVertex3d(lon0, lat0 - 1, 0.0);
  glEnd();
  glPopMatrix();
}

void draw_polyline()
{
  glPushMatrix();
  for (int i = 1; i<int(polyline.size()); i++) 
  {
    if (polyline[i].n2c == 6) glColor3d(0.4, 0.7, 0.7);
    else                      glColor3d(0.4, 0.4, 0.4);
    glBegin(GL_LINE_STRIP);
    for (int j = 0; j < polyline[i].nPoints; j++)
      glVertex3d(polyline[i].points[j].lon, polyline[i].points[j].lat, 0.0);
    glEnd();
  }
  glPopMatrix();
  glColor3d(1.0, 1.0, 1.0);
}

void draw_scene()
{
  reDraw = false;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glTranslated(lon0, lat0, 0.0);
  glScaled(alfa_zoom, alfa_zoom, 1.0);
  glTranslated(-lon0, -lat0, 0.0);
  glTranslated(delta_lon, delta_lat, 0.0);
  glPushMatrix();
  draw_polyline();
  disegna_prova();
  glPopMatrix();
  glPopMatrix();
}

class Frame : public Fl_Gl_Window 
{
  void draw()
  {
    if (!valid()) 
    {
      glClearColor(0.0, 0.0, 0.0, 1);                        // Turn the background color black
      glViewport(0, 0, w(), h());                            // Make our viewport the whole window
      glMatrixMode(GL_PROJECTION);                           // Select The Projection Matrix
      glLoadIdentity();                                      // Reset The Projection Matrix
      gluOrtho2D(lon0 - dlon, lon0 + dlon, lat0 - dlat, lat0 + dlat);
      glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
      glLoadIdentity();                                      // Reset The Modelview Matrix
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear The Screen And The Depth Buffer
      glLoadIdentity();                                      // Reset The View
      glEnable(GL_DEPTH_TEST);
      valid(1);
    }
    draw_scene();
  }

  int handle(int) { return 1; };

public:
  Frame(int x, int y, int w, int h, const char *l = 0) : Fl_Gl_Window(x, y, w, h, l) 
  {
    alfa_zoom = 1.0;
    delta_lon = 0.0;
    delta_lat = 0.0;
  }
};

void CreateMyWindow()
{
  // Get screen resolution and adjust window dimension to 90% of width and height
  int form_x, form_y, form_w, form_h;
  Fl::screen_work_area(form_x, form_y, form_w, form_h, 0);
  int w_est, h_est;
  w_est = int(form_w * 0.9);
  h_est = int(form_h * 0.9);
  screen_w = int(w_est * 0.8);
  screen_h = int(h_est * 0.8);
  int spacing = 25;

  form = new Fl_Window(spacing, spacing, w_est, h_est, "Flux Box");
  scene = new Frame(spacing, spacing, screen_w, screen_h, 0);

  form->end();
  form->show();
  scene->show();
}

int main()
{
  Geometria();
  CreateMyWindow();
  Fl::add_idle([](void*)
  {
    if (reDraw) scene->redraw();
  }, 0);

  return Fl::run();
}
