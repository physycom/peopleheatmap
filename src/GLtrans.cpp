//--------------------------------------------------------------------------------------------------
// ** Uso **
//    GLdouble objx,objy,objz,winx,winy,winz;
//    void SaveModel(void);
//    ierr=  Project( objx,objy,objz,&winx,&winy,&winz );
//    ierr=  UnProject( winx,winy,winz,&objx,&objy,&objz );
//--------------------------------------------------------------------------------------------------
#include <common.h>
#include "GLtrans.h"
#include <FL/glu.h>                                        // Header File For The GLu32    Library

GLint viewport[4];
GLdouble modelMatrix[16], projMatrix[16];
extern int  screen_w, screen_h;

//--------------------------------------------------------------------------------------------------
void SaveModel(void){
    glGetDoublev(  GL_MODELVIEW_MATRIX,  modelMatrix );
    glGetDoublev(  GL_PROJECTION_MATRIX, projMatrix  );
    glGetIntegerv( GL_VIEWPORT,          viewport    );
	screen_w = viewport[2];
	screen_h = viewport[3];
}
//--------------------------------------------------------------------------------------------------
int Project(GLdouble objx,GLdouble objy,GLdouble objz,GLdouble *winx,GLdouble *winy,GLdouble *winz)
{
    int ierr;
    ierr=  gluProject( objx, objy, objz, modelMatrix, projMatrix, viewport, winx, winy, winz );
    return ierr;
}
//--------------------------------------------------------------------------------------------------
int UnProject(GLdouble winx,GLdouble winy,GLdouble winz,GLdouble *objx,GLdouble *objy,GLdouble *objz)
{
    int ierr;
    ierr=  gluUnProject( winx,winy,winz, modelMatrix, projMatrix, viewport, objx, objy, objz );
    return ierr;
}
//--------------------------------------------------------------------------------------------------


