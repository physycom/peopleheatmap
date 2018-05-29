#include "stdafx.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Bar.H>
#include "dati_local.h"
#include "leggi_dati.h"
#include "frame.h"
#include "form.h"
#include "draw.h"
#include "ale/subnet_gra.h"

Fl_Window       *form;
Frame           *scene = NULL;
Fl_Value_Slider *zoom;
Fl_Output       *linea1, *linea2;
Fl_Button       *button01;
Fl_Check_Button *button1, *button2, *button3;
Fl_Check_Button *button32;

extern list <int> path; extern int n_people, n_traj;
extern int poly_evidenziata;

int screen_w = SCREEN_WIDTH, screen_h = SCREEN_HEIGHT;
int w_est, h_est, space, b_w, b_h;
int offset_h, r_offset_h;

bool reDraw = true;
bool mostraPoly = false, mostraNodo = false, mostraGrid = true;
bool mostraSubnet = false;
double alfa_zoom = ZOOM_START, tempo_value = 0.0;
string subnet_select;

//-------------------------------------------------------------------------------------------------
void set_off() {
  button1->clear(); button2->clear(); button3->clear();
  button32->clear();
  mostraPoly = mostraNodo = false;
  mostraSubnet = mostraGrid = false;
  reDraw = true; linea2->value("");
}

void exit_cb(Fl_Widget*) { form->hide(); }
void zoom_cb(Fl_Widget*) { alfa_zoom = zoom->value()*zoom->value(); reDraw = true; }
void MostraPoly_cb(Fl_Widget*) { set_off();      button1->set(); mostraPoly = !mostraPoly; }
void MostraNodo_cb(Fl_Widget*) { set_off();      button2->set(); mostraNodo = !mostraNodo; }
void MostraGrid_cb(Fl_Widget*) { set_off();      button3->set(); mostraGrid = !mostraGrid; }
//-------------------------------------------------------------------------------------------------
void CreateMyWindow(void) {   

  // Get screen resolution and adjust window dimension to 90% of screen width and height
  int form_x, form_y, form_w, form_h;
  Fl::screen_work_area(form_x, form_y, form_w, form_h, 0);
  w_est = int(form_w * 0.9);
  h_est = int(form_h * 0.9);
  screen_w = int(w_est * 0.8);
  screen_h = int(h_est * 0.8);
  space = int(0.01 * w_est);
  b_w = int(0.08 * w_est);
  b_h = int(0.04 * h_est);

  form = new Fl_Window(10, 10, w_est, h_est, "");
  new Fl_Box(FL_DOWN_FRAME, space + b_w + space - 3, space - 3, screen_w + 6, screen_h + 6, "SR");
  scene = new Frame(space + b_w + space, space, screen_w, screen_h, 0);

  // ------------  Linee di testo  ----------------------------------------------------
  linea1 = new Fl_Output(space, 2 * space + screen_h, screen_w / 3, b_h, "");
  linea2 = new Fl_Output(2 * space + screen_w / 3, 2 * space + screen_h, 2 * screen_w / 3 - space, b_h, ""); 
  // ------------  Zoom ---------------------------------------------------------------
  zoom = new Fl_Value_Slider(space, h_est - 2 * space - b_h, 4 * b_w, b_h, "Zoom");
  zoom->type(FL_HOR_SLIDER);  zoom->bounds(0.5, 10.0); zoom->value(ZOOM_START);      zoom->callback(zoom_cb);
  alfa_zoom = zoom->value()*zoom->value();
 
  // ------------  Exit --------------------------------------------------------------- 
  button01 = new Fl_Button(w_est - space - b_w, h_est - space - b_w, b_w, b_w, "Exit");
  button01->callback(exit_cb);
  // ------------  Explorer - -------------------------------------------------------
  offset_h = space;
  // ------------  Subnet  ---------------------------------------------------------
  CreaSubnet();
  // ------------  CalcPath --------------------------------------------------------
  r_offset_h = space;
  button1 = new Fl_Check_Button(4 * space + screen_w + b_w, r_offset_h, b_w, b_h, "Poly"); r_offset_h += b_h;
  button1->callback(MostraPoly_cb);
  if (mostraPoly) button1->set();
  button2 = new Fl_Check_Button(4 * space + screen_w + b_w, r_offset_h, b_w, b_h, "Nodo"); r_offset_h += b_h;
  button2->callback(MostraNodo_cb);
  if (mostraNodo) button2->set();
  button3 = new Fl_Check_Button(4 * space + screen_w + b_w, r_offset_h, b_w, b_h, "Grid"); r_offset_h += b_h;
  button3->callback(MostraGrid_cb);
  if (mostraGrid) button3->set();

  //-------------------------------------------------------------------------------	
  form->end();
  form->show();
  scene->show();
}
//-------------------------------------------------------------------------------------------------

