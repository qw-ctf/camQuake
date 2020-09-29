#ifdef CAMQUAKE
#ifndef _CAMQUAKE
#define _CAMQUAKE
#include "../keys.h"
void Camquake_Init(void);
void Camquake_Frame(void);
void Camquake_Render_Frame(void);
void Camquake_Setup_Projection(void);
qbool Camquake_MouseEvent(mouse_state_t *ms);
qbool Camquake_KeyEvent(int key, int unichar, qbool down);
void Camquake_2D_Draw(void);
#endif
#endif
