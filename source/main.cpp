#include "stdafx.h"
#include <FL/Fl.H>
#include "leggi_dati.h"
#include "metodi.h"
#include "form.h"
#include "frame.h"
#include "ale/subnet_gra.h"
#include "mappa.h"
#include "Globalmaptiles.h"

using namespace std;

extern Frame *scene;
extern bool   reDraw;
extern map<pair<int, int>, MapTile> grid_map;
string File;
// Main ----------------------------------------------------------------------------------------
void idle_cb(void*)
{
  if (reDraw) scene->redraw();
}
//-------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
  bool enable_fltk;
  if (argc == 2) {
    File = argv[1];
    enable_fltk = false;
  }
  else if (argc == 3) {
    File = argv[1];
    enable_fltk = true;
  }
  else {
    cerr << "Usage : " << argv[0] << " path/to/data/csv [enable_fltk]" << endl;
    exit(1);
  }
  cout << "Input csv : " << File << endl;


  Geometria();
  LeggiPoly("Venezia_SM");
  CreaNodi();
  CreaMappa();
  ImportaSubnet("../output/subnet.txt");
  make_grid_map();
  save_grid_heatmap("test.png");
  if (enable_fltk)
  {
    CreateMyWindow();
    Fl::add_idle(idle_cb, 0);
    Fl::run();
  }

  return 0;
}
//-------------------------------------------------------------------------------------------------

