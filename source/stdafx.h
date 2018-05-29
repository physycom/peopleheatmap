// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequelnty
//

#pragma once
//#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include "targetver.h"
#include <tchar.h>
#endif

#include <iostream>
#include <iomanip>      // std::setprecision
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include "dati_local.h"

#ifdef _WIN32
#define PAU system("Pause")
#else
#define PAU cout << "Hit ENTER to continue" << endl; cin.get();
#endif

using namespace std;


// TODO: reference additional headers your program requires here
