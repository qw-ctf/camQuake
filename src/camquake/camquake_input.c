#include "../quakedef.h"
#include "../keys.h"
#include "camquake_internal.h"

extern struct camquake *camquake;

void Camquake_Change_Movement_Axis(int direction) {
	camquake->edit.movement_axis += direction;
	if (camquake->edit.movement_axis > CQEM_MA_MAX) {
		camquake->edit.movement_axis = CQEM_MA_MIN;
	}
	if (camquake->edit.movement_axis < CQEM_MA_MIN) {
		camquake->edit.movement_axis = CQEM_MA_MAX;
	}
}

void Camquake_Change_Select_Mode(int direction) {
	camquake->edit.select_mode += direction;
	if (camquake->edit.select_mode> CQEM_SELECT_MODE_MAX) {
		camquake->edit.select_mode = CQEM_SELECT_MODE_MIN;
	}
	if (camquake->edit.select_mode < CQEM_SELECT_MODE_MIN) {
		camquake->edit.select_mode= CQEM_SELECT_MODE_MAX;
	}
}


void Camquake_Insert_Segment_Point(void) {
	float t;
	int i;
	struct camquake_path_point p, *np;
	struct camquake_path_point_array *path;
	p.x = 0;
	p.y = 0;
	p.z = 0;
	if (camquake->selected_segment.valid == 0) {
		return;
	}
	if (camquake->selected_path == NULL) {
		return;
	}
	t = (camquake->selected_segment.p0->time + camquake->selected_segment.p1->time) / 2;
	p.time = t;
	path = camquake->selected_path->path;
	t -= path->point[0].time;
	CQS_Interpolate_Path(camquake->selected_path->path, t, &p);
	for (i=0; i<path->index;i++) {
		if (path->point[i].time > p.time) {
			break;
		}
	}
	np = CQP_Point_New_Index(&camquake->selected_path->path, i);
	memcpy(np, &p, sizeof*np);
	camquake->selected_path = NULL;
	camquake->selected_point = NULL;
	camquake->selected_segment.valid = 0;
	camquake->selected_setup->changed = 1;
}

void Camquake_Find_Highlight (struct camquake_path_point *cursor) {
	struct camquake_setup *setup;
	struct camquake_path_point *p;
	struct camquake_path *path;
	float distance = 10;
	float d;
	int i;

	setup = camquake->selected_setup;
	if (setup == NULL) {
		return;
	}

	camquake->selected_point = NULL;
	camquake->selected_path = NULL;
	path = &setup->camera_path;
	if (path->projected_points != NULL)
	{
		for (i=0, p=path->projected_points; i<path->path->index; i++, p++)
		{
			if (p->z == -1)
				continue;
			d = VectorDistance(&p->x, &cursor->x);
			if (d < distance) {
				camquake->selected_point = &path->path->point[i];;
				camquake->selected_path = path;
				camquake->selected_point_p = p;

				if (path->path->index < 2) {
					camquake->selected_segment.valid = 0;
				} else {
					camquake->selected_segment.valid = 1;
				camquake->selected_segment.p0 = &path->path->point[i];
				if (i < path->path->index - 1) {
					camquake->selected_segment.p1 = &path->path->point[i+1];
				} else {
					camquake->selected_segment.p1 = camquake->selected_segment.p0; 
					camquake->selected_segment.p0 = &path->path->point[i-1];
				}

				distance = d;
			}
		}
	}

	path = &setup->view_path;
	if (path->projected_points != NULL)
	{
		for (i=0, p=path->projected_points; i<path->path->index; i++, p++)
		{
			if (p->z == -1)
				continue;
			d = VectorDistance(&p->x, &cursor->x);
			if (d < distance) {
				camquake->selected_point = &path->path->point[i];;
				camquake->selected_path = path;
				camquake->selected_point_p = p;
				distance = d;

				if (path->path->index < 2) {
					camquake->selected_segment.valid = 0;
				} else {
					camquake->selected_segment.valid = 1;
					if (i < path->path->index - 1) {
						camquake->selected_segment.p1 = &path->path->point[i+1];
					} else {
						camquake->selected_segment.p1 = camquake->selected_segment.p0; 
						camquake->selected_segment.p0 = &path->path->point[i-1];
					}
				}
			}
		}
	}
	}
}

void Camquake_Change_Time(float x, float y, struct camquake_path_point *point, struct camquake_path *path) {
	int i;
	if (x == 0 && y == 0 )
		return;

	if (camquake->edit.select_mode == CQEM_SELECT_MODE_PATH) {
		for (i=0<i;path->path->index; i++) {
			path->path->point[i].time += x;
		}
	} else if (camquake->edit.select_mode== CQEM_SELECT_MODE_SEGMENT) {
		camquake->selected_segment.p0->time += x;
		camquake->selected_segment.p1->time += x;
	} else {
		point->time += x;
	}
}

void Camquake_Move_Path(float x, float y, struct camquake_path_point *point, struct camquake_path *path) {
	struct camquake_path_point offset, *cp;
	int i;
	vec3_t forward, up, right;
	if (x == 0 && y == 0 )
		return;

	memset(&offset, 0, sizeof(offset));

	switch(camquake->edit.movement_axis) {
		case CQEM_MA_XY:
			offset.x += x;
			offset.y += y;
			break;
		case CQEM_MA_XZ:
			offset.x += x;
			offset.z += y;
			break;
		case CQEM_MA_YZ:
			offset.y += x;
			offset.z += y;
			break;
		case CQEM_MA_X:
			offset.x += x + y;
			break;
		case CQEM_MA_Y:
			offset.y += x + y;
			break;
		case CQEM_MA_Z:
			offset.z += x + y;
			break;
		case CQEM_MA_VIEW:
			AngleVectors(r_refdef.viewangles, forward, right, up);
			VectorScale(right, x, right);
			VectorAdd(&offset.x, right, &offset.x);
			VectorScale(up, y, up);
			VectorAdd(&offset.x, up, &offset.x);
			break;
	}

	if (camquake->edit.select_mode== CQEM_SELECT_MODE_PATH) {
		if (path) {
			for (i=0, cp = &path->path->point[0]; i<path->path->index; i++, cp++) {
				VectorAdd(&cp->x, &offset.x, &cp->x);
			}
		}
	} else if (camquake->edit.select_mode== CQEM_SELECT_MODE_SEGMENT) {
		if (camquake->selected_segment.valid) {
			cp = camquake->selected_segment.p0;
			VectorAdd(&cp->x, &offset.x, &cp->x);
			cp = camquake->selected_segment.p1;
			VectorAdd(&cp->x, &offset.x, &cp->x);
		}
	} else {
		VectorAdd(&point->x, &offset.x, &point->x);
	}
}


qbool Camquake_MouseEvent(mouse_state_t *ms)
{
	struct camquake_path_point cursor;
	float x, y;
	cursor.x = ms->x;
	cursor.y = ms->y;
	cursor.z = 0;

	if (ms->button_up == 1 || !ms->buttons[1]) {
		camquake->selected_point = NULL;
	}

	if (camquake->selected_point) {
		if (ms->buttons[1]) {
			if (ms->button_down == 1) {
				camquake->edit.cursor_initial.x = ms->x_old;
				camquake->edit.cursor_initial.y = ms->y_old;
				camquake->edit.cursor_old.x = ms->x;
				camquake->edit.cursor_old.y = ms->y;
			}
			x = ms->x - camquake->edit.cursor_old.x;
			x = x * cls.trueframetime * camquake->edit.movement_multiplier * 100;
			y = ms->y - camquake->edit.cursor_old.y;
			y = y * cls.trueframetime * camquake->edit.movement_multiplier * 100;
			if (camquake->edit.edit_mode == 0) {
				Camquake_Move_Path(x, y, camquake->selected_point, camquake->selected_path); 
			} else {
				Camquake_Change_Time(x, y, camquake->selected_point, camquake->selected_path); 
			}
			camquake->setup_projection = 1;
			camquake->selected_setup->changed = 1;
			camquake->edit.cursor_old.x = ms->x;
			camquake->edit.cursor_old.y = ms->y;

			ms->x = ms->x_old = camquake->edit.cursor_initial.x; 
			ms->y = ms->y_old = camquake->edit.cursor_initial.y; 
			return true;
		}
	} else {
		camquake->edit.cursor_initial.x = 0;
		camquake->edit.cursor_initial.y = 0;
		Camquake_Find_Highlight(&cursor);
	}
	return true;
}

qbool Camquake_KeyEvent(int key, int unichar, qbool down)
{
	switch (key)
	{
		case K_ESCAPE :
			Camquake_InputRelease();
			return true;
		case '1':
			Camquake_Change_Movement_Axis(-1);
			return true;
		case '2':
			Camquake_Change_Movement_Axis(1);
			return true;
		case '3':
			Camquake_Change_Select_Mode(1);
			return true;
		case '4':
			camquake->edit.movement_multiplier *= 0.1;
			return true;
		case '5':
			camquake->edit.movement_multiplier *= 10.0;
			return true;
		case K_F1:
			camquake->edit.help = !camquake->edit.help;
			return true;
		case K_TAB:
			camquake->edit.edit_mode = !camquake->edit.edit_mode;
			return true;
		case K_INS:
			Camquake_Insert_Segment_Point();
			return true;
	}
	return true;
}
