#pragma once

#include <FL/gl.h>

void SaveModel(void);
int Project(GLdouble objx,GLdouble objy,GLdouble objz,GLdouble *winx,GLdouble *winy,GLdouble *winz);
int UnProject(GLdouble winx,GLdouble winy,GLdouble winz,GLdouble *objx,GLdouble *objy,GLdouble *objz);
