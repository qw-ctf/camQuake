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
	struct camquake_path_point *p;

	if (path->interpolated_array) {
		free(path->interpolated_array);
	}
	path->interpolated_array = calloc(INTERPOLATION_COUNT, sizeof(*path->interpolated_array));
	if (path->interpolated_array == NULL) {
		return;
	}
	for (i=0, t=0; i<INTERPOLATION_COUNT; i++, t+=0.01) {
		p = &path->interpolated_array[i];
		CQS_Interpolate_Path(path->path, t, p);
	}
}

void Camquake_Render_Movement_Directions(struct camquake_path_point *p) {
	struct camquake_path_point stop_mouse_x, stop_mouse_y;
	float line_length = 50;
	vec3_t forward, up, right;

	memcpy(&stop_mouse_x, p, sizeof(*p));
	memcpy(&stop_mouse_y, p, sizeof(*p));
	if (p == NULL) {
		return;
	}
	switch (camquake->edit.movement_axis) {
		case CQEM_MA_XY:
			stop_mouse_x.x += line_length;
			stop_mouse_y.y += line_length;
			break;
		case CQEM_MA_XZ:
			stop_mouse_x.x += line_length;
			stop_mouse_y.z += line_length;
			break;
		case CQEM_MA_YZ:
			stop_mouse_x.y += line_length;
			stop_mouse_y.z += line_length;
			break;
		case CQEM_MA_X:
			stop_mouse_x.x += line_length;
			stop_mouse_y.x += line_length;
			break;
		case CQEM_MA_Y:
			stop_mouse_x.y += line_length;
			stop_mouse_y.y += line_length;
			break;
		case CQEM_MA_Z:
			stop_mouse_x.z += line_length;
			stop_mouse_y.z += line_length;
			break;
		case CQEM_MA_VIEW:
			AngleVectors(r_refdef.viewangles, forward, right, up);
			VectorScale(right, line_length, right);
			VectorAdd(&stop_mouse_x.x, right, &stop_mouse_x.x);
			VectorScale(up, line_length, up);
			VectorAdd(&stop_mouse_y.x, up, &stop_mouse_y.x);
			break;
	}
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
	if (path == NULL) {
		return;
	}
	if (path->path == NULL) {
		return;
	}

	glDisable(GL_TEXTURE_2D);
	glLineWidth(4);
	if (camquake->selected_path == path && camquake->edit.move_path == 1) {
		Camquake_Custom_Color(&camquake->colors.highlight_path);
	} else {
		Camquake_Custom_Color(&color_curve);
	}
	GLC_Begin(GL_LINE_STRIP);
	for (i=0; i<INTERPOLATION_COUNT; i++) {
		GLC_Vertex3fv((GLfloat *)&(path->interpolated_array)[i]);
	}
	GLC_End();

	Camquake_Custom_Color(&color_points);
	glPointSize(20);
	GLC_Begin(GL_POINTS);
	for (i=0; i<path->path->index; i++) {
		if (&path->path->point[i] == camquake->selected_point) {
			Camquake_Custom_Color(&camquake->colors.highlight_point);
		}
		GLC_Vertex3fv((GLfloat *)&(path->path->point[i]));
		if (&path->path->point[i] == camquake->selected_point) {
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
	if (camquake->selected_point != NULL && camquake->have_input) {
		Camquake_Render_Movement_Directions(camquake->selected_point);
	}

}
