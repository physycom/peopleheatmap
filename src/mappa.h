#pragma once

#include <common.h>
#include "polyline.h"

//enum Caso{ A, V, T};

//----------------------------------------------------------------------------
void CreaMappa(void);
void CreaSegmenti(void);
int  CercaPolyVicina(double x, double y, double &distanza, int &id_poly);
bool CercaNodoVicino(double x, double y, double &distanza, int &id_nodo);
//----------------------------------------------------------------------------


