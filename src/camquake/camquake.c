#include "../quakedef.h"
#include "../keys.h"
#include "camquake_internal.h"

extern int glwidth, glheight;

struct camquake *camquake;


struct camquake_color_variable_default {
	char *name, *value;
};

struct camquake_color_variable_default camquake_color_variables_defaults[] = {
	{"camquake_color_camera_path", ".75 .25 .25"},
	{"camquake_color_camera_point", "1 .14 .14"},
	{"camquake_color_view_path", ".25 .75 .25"},
	{"camquake_color_view_point", ".14 1 .14"},
	{"camquake_color_highlight_path", "0 0 1"},
	{"camquake_color_highlight_point", "0 0 1"},
	{"camquake_color_movement_x", "1 0 0"},
	{"camquake_color_movement_y", "0 1 0"},
};

void Camquake_Color_Change(struct cvar_s *var, char *value, qbool *cancel) {
	struct camquake_color_variable_default *color_default;
	int offset;
	char buf[1024];
	char *result;
	int i;
	strlcpy(buf, value, sizeof(buf));

	for (offset=0, color_default=&camquake_color_variables_defaults[0]; color_default!= NULL;offset++, color_default++) {
		if (strcmp(var->name, color_default->name) == 0){
			break;
		}
	}
	if (color_default == NULL) {
		Com_Printf("color: %s not found\n", var->name);
		*cancel = 1;
		return;
	}

	result = strtok(buf, " ");
	for (i=0; i<3; i++, result = strtok(NULL, " ")) {
		if (result == NULL) {
			Com_Printf("needed format: %s r g b\n", var->name);
			*cancel = 1;
			return;
		}
		*((&(((struct color4f *)&camquake->colors) + offset)->r) + i) = Q_atof(result);
	}
	return;
}

inline int Point_Infront(struct camquake_path_point *p, vec3_t forward) {
	if (DotProduct(forward, (vec_t *) p) < 0) {
		return 0;
	}
	return 1;
}


/*
vector intersectionPoint(vector lineVector, vector linePoint, vector planeNormal, vector planePoint){
	vector diff = subVectors(linePoint,planePoint);
 
	v1 = addVectors(diff,planePoint)
	scale = -dotProduct(diff,planeNormal)/dotProduct(lineVector,planeNormal)
	v2 = scaleVector(scale ,lineVector)
	intersection = addVectors(v1, v2);
	*/

float Point_Distance(vec3_t forward, vec3_t origin, vec3_t point) {
	vec3_t intersection, diff, v1, v2, v3;
	VectorSubtract(origin, point, diff);
	VectorAdd(diff, point, v1);
	float scale = -DotProduct(diff, forward)/DotProduct(forward, forward);
	// WTF VECTORSCALE???
	v2[0] = forward[0] * scale;
	v2[1] = forward[1] * scale;
	v2[2] = forward[2] * scale;
	VectorAdd(v1, v2, intersection);
	//VectorCopy(intersection, &camquake->selected_point.x);
	VectorSubtract(intersection, point, v3);
	return VectorLength(v3);
}

void Camquake_InputGrab(void) {
	key_dest = key_camquake;
	camquake->have_input = 1;
}

void Camquake_InputRelease(void) {
	key_dest = key_game;
	key_dest_beforecon = key_game;
	camquake->have_input = 0;
}

/*
void Camquake_Find_Highlight (void) {
	struct camquake_setup *setup;
	struct camquake_path_point *p;
	vec3_t forward, up, right;
	float distance = 100;
	float d;
	int i;

	setup = camquake->active_setup;
	if (setup == NULL) {
		camquake->selected_setup = NULL;
		return;
	}

	if (setup->camera_path.path == NULL) {
		return;
	}

	AngleVectors(r_refdef.viewangles, forward, right, up);

	camquake->selected_point = NULL;
	for (i=0, p=&setup->camera_path.path->point[0]; i<setup->camera_path.path->index; i++, p++)
	{
		d = Point_Distance(forward, r_refdef.vieworg, &p->x);
		if (d < distance) {
			Com_Printf("we selected a point\n");
			camquake->selected_point = p;
			distance = d;
		}
	}

	if (setup->camera_path.path->index > 0)
	{
	}

	if (setup->view_path.path->index > 0)
	{
	}
}
*/

void Camquake_Init(void)
{
	int offset;
	struct camquake_color_variable_default *cd;
	cvar_t *cd_var;
	camquake = calloc(1, sizeof(*camquake));
	if (camquake == NULL) {
		return;
	}
	camquake->enabled.name = "camquake_enabled";
	camquake->enabled.string = "0";
	Cvar_Register(&camquake->enabled);
	camquake->overwrite_on_load.name = "camquake_overwrite_on_load";
	camquake->overwrite_on_load.string = "0";
	Cvar_Register(&camquake->overwrite_on_load);
	camquake->render.name = "camquake_render";
	camquake->render.string = "0";
	Cvar_Register(&camquake->render);

	for (offset=0, cd=&camquake_color_variables_defaults[0], cd_var=(cvar_t *)&camquake->color_variables;
			offset < sizeof(struct camquake_color_variables)/sizeof(cvar_t);
			cd++, offset++, cd_var++) {
		cd_var->name = cd->name;
		cd_var->string = cd->value;
		cd_var->OnChange = Camquake_Color_Change;
		Cvar_Register(cd_var);
		Cvar_ForceCallback(cd_var);
		camquake->edit.movement_multiplier = 1;
	}

	Cmd_AddCommand("camquake", Camquake_Cmd);
}

void Camquake_Frame(void)
{
	float t = 0;
	camquake->current_time += cls.trueframetime;
	if (camquake->active_setup) {
		if (camquake->current_time >= camquake->active_setup->time_start &&
		camquake->current_time <= camquake->active_setup->time_stop) {
			t = (camquake->current_time - camquake->active_setup->time_start) / (camquake->active_setup->time_stop - camquake->active_setup->time_start);

			CQS_Interpolate(camquake->active_setup, t, (struct camquake_path_point *)&r_refdef.vieworg, (struct camquake_path_point *)&r_refdef.viewangles);
		}
	}
}

void Camquake_Render_Frame(void)
{
	if (camquake->render.value != 1) {
		return;
	}

	if (camquake->selected_setup) {
		Camquake_Render_Setup(camquake->selected_setup);
	}
}

void Camquake_Setup_Projection (void) {
	if (camquake->selected_setup) {
		if (camquake->have_input == 1 || camquake->setup_projection == 1) {
			Camquake_Render_Setup_Projected_Points(&camquake->selected_setup->camera_path);
			Camquake_Render_Setup_Projected_Points(&camquake->selected_setup->view_path);
			camquake->have_input = 2;
			camquake->setup_projection = 0;
		}
	}
}

void Camquake_Exit(void)
{

}

