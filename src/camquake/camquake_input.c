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
			}
		}
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

	if (camquake->edit.move_path == 1) {
		if (path) {
			for (i=0, cp = &path->path->point[0]; i<path->path->index; i++, cp++) {
				VectorAdd(&cp->x, &offset.x, &cp->x);
			}
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
	if (ms->buttons[1]) {
		if (camquake->selected_point) {
			x = (ms->x - ms->x_old) * cls.frametime * camquake->edit.movement_multiplier;
			y = (ms->y - ms->y_old) * cls.frametime * camquake->edit.movement_multiplier;
			Camquake_Move_Path(x, y, camquake->selected_point, camquake->selected_path); 
			ms->x = ms->x_old;
			ms->y = ms->y_old;
			camquake->setup_projection = 1;
			camquake->selected_setup->changed = 1;
		}
		return true;
	} else {
		Camquake_Find_Highlight(&cursor);
	}
	return false;
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
			camquake->edit.move_path = !camquake->edit.move_path;
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
	}
	return true;
}
