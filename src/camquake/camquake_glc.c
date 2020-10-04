#include "camquake_internal.h"
#include "../gl_local.h"
#include "../glc_local.h"
#include "../r_state.h"

extern struct camquake *camquake;
qbool R_Project3DCoordinates(float objx, float objy, float objz, float* winx, float* winy, float* winz);

#define INTERPOLATION_COUNT 100

void Camquake_Custom_Color(struct color4f *color) {
	R_CustomColor(color->r, color->g, color->b, color->a);
}

void Camquake_Render_Setup_Projected_Points(struct camquake_path *path) {
	struct camquake_path_point *p, *out;
	int i;

	if (path == NULL) {
		return;
	}

	if (path->projected_points) {
		free(path->projected_points);
		path->projected_points = NULL;
	}

	if (path->path == NULL) {
		return;
	}

	if (path->path->index > 0) {
		path->projected_points = calloc(path->path->index, sizeof(*p));
	}

	if (path->projected_points == NULL) {
		return;
	}

	for (i=0,  p = &path->path->point[0], out = path->projected_points; i<path->path->index;i++, p++, out++) {
		if (R_Project3DCoordinates(p->x, p->y, p->z, &out->x, &out->y, &out->z)< 0) {
			out->z = -1;
		} else {
		out->x = out->x * vid.width / glwidth;
		out->y = (glheight - out->y) * vid.height / glheight;
		out->z = 0;
		}
	}
}

void Camquake_Render_Update_Path(struct camquake_path *path) {
	int i;
	float t;
	float dt;
	struct camquake_path_point *p;

	if (path->interpolated_array) {
		free(path->interpolated_array);
	}

	if (path->path == NULL) {
		return;
	}

	if (path->path->index < 2) {
		return;
	}

	path->interpolated_array = calloc(INTERPOLATION_COUNT, sizeof(*path->interpolated_array));
	if (path->interpolated_array == NULL) {
		return;
	}
	dt = path->path->point[path->path->index-1].time - path->path->point[0].time;

	for (i=0; i<INTERPOLATION_COUNT; i++) {
		p = &path->interpolated_array[i];
		t = ( dt / INTERPOLATION_COUNT) * i;
		CQS_Interpolate_Path(path->path, t, p);
	}
}

void Camquake_Render_Setup_Direction_Lines(struct camquake_path_point *p, struct camquake_path_point *p0, struct camquake_path_point *p1) {
	float line_length = 50;
	memcpy(p0, p, sizeof(*p));
	memcpy(p1, p, sizeof(*p));
	vec3_t forward, up, right;
	if (camquake->edit.edit_mode == 0) {
		switch (camquake->edit.movement_axis) {
			case CQEM_MA_XY:
				p0->x += line_length;
				p1->y += line_length;
				break;
			case CQEM_MA_XZ:
				p0->x += line_length;
				p1->z += line_length;
				break;
			case CQEM_MA_YZ:
				p0->y += line_length;
				p1->z += line_length;
				break;
			case CQEM_MA_X:
				p0->x += line_length;
				p1->x += line_length;
				break;
			case CQEM_MA_Y:
				p0->y += line_length;
				p1->y += line_length;
				break;
			case CQEM_MA_Z:
				p0->z += line_length;
				p1->z += line_length;
				break;
			case CQEM_MA_VIEW:
				AngleVectors(r_refdef.viewangles, forward, right, up);
				VectorScale(right, line_length, right);
				VectorAdd(*((vec3_t *)&p0->x), right, *((vec3_t *)&p0->x));
				VectorScale(up, line_length, up);
				VectorAdd(*((vec3_t *)&p1->x), up, *((vec3_t *)&p1->x));
				break;
		}
	}
}

void Camquake_Render_Movement_Directions(struct camquake_path_point *p) {
	struct camquake_path_point stop_mouse_x, stop_mouse_y;

	if (p == NULL) {
		return;
	}

	Camquake_Render_Setup_Direction_Lines(p, &stop_mouse_x, &stop_mouse_y);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(4);
	GLC_Begin(GL_LINE_STRIP);
	Camquake_Custom_Color(&camquake->colors.movement_x);
	GLC_Vertex3fv((GLfloat *)&p->x);
	GLC_Vertex3fv((GLfloat *)&stop_mouse_x.x);
	GLC_End();
	GLC_Begin(GL_LINE_STRIP);
	Camquake_Custom_Color(&camquake->colors.movement_y);
	GLC_Vertex3fv((GLfloat *)&p->x);
	GLC_Vertex3fv((GLfloat *)&stop_mouse_y.x);
	GLC_End();
	glEnable(GL_TEXTURE_2D);
}

void Camquake_Render_Path(struct camquake_path *path, struct color4f color_curve, struct color4f color_points) {
	int i;
	int clear_color;
	if (path == NULL) {
		return;
	}
	if (path->path == NULL) {
		return;
	}
	clear_color = 0;

	glDisable(GL_TEXTURE_2D);
	glLineWidth(4);
	if (camquake->selected_path == path && camquake->edit.select_mode== CQEM_SELECT_MODE_PATH) {
		Camquake_Custom_Color(&camquake->colors.highlight_path);
	} else {
		Camquake_Custom_Color(&color_curve);
	}
	if (path->interpolated_array != NULL) {
		GLC_Begin(GL_LINE_STRIP);
		for (i=0; i<INTERPOLATION_COUNT; i++) {
			GLC_Vertex3fv((GLfloat *)&(path->interpolated_array)[i]);
		}
		GLC_End();
	}

	Camquake_Custom_Color(&color_points);
	glPointSize(20);
	GLC_Begin(GL_POINTS);
	for (i=0; i<path->path->index; i++) {
		if (
		(&path->path->point[i] == camquake->selected_point && camquake->edit.select_mode == CQEM_SELECT_MODE_POINT) ||
		((&path->path->point[i] == camquake->selected_segment.p0 || &path->path->point[i] == camquake->selected_segment.p1) && camquake->edit.select_mode == CQEM_SELECT_MODE_SEGMENT)) {
			Camquake_Custom_Color(&camquake->colors.highlight_point);
			clear_color = 1;
		}
		GLC_Vertex3fv((GLfloat *)&(path->path->point[i]));
		if (clear_color) {
			clear_color = 0;
			Camquake_Custom_Color(&color_points);
		}
	}
	GLC_End();
	glEnable(GL_TEXTURE_2D);
}

void Camquake_Render_Setup (struct camquake_setup *setup) {
	if (setup->changed == 1 || setup->camera_path.interpolated_array == NULL) {
		Camquake_Render_Update_Path(&setup->camera_path);
		Camquake_Render_Update_Path(&setup->view_path);
		setup->changed = 0;
		// stupid needs to change
	}

	Camquake_Render_Path(&setup->camera_path, camquake->colors.camera_path, camquake->colors.camera_point);
	Camquake_Render_Path(&setup->view_path, camquake->colors.view_path, camquake->colors.view_point);
	if (camquake->have_input) {
		if (camquake->selected_point != NULL && (camquake->edit.select_mode == CQEM_SELECT_MODE_POINT || camquake->edit.select_mode == CQEM_SELECT_MODE_PATH)) {
			Camquake_Render_Movement_Directions(camquake->selected_point);
		} else if (camquake->edit.select_mode == CQEM_SELECT_MODE_SEGMENT) {
			if (camquake->selected_segment.valid == 1) {
				Camquake_Render_Movement_Directions(camquake->selected_segment.p0);
				Camquake_Render_Movement_Directions(camquake->selected_segment.p1);
			}
		}
	}

}
