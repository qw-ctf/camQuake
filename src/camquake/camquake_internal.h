#include "../quakedef.h"

struct camquake_path_point {
	float x, y, z;
	float time;
};

struct camquake_path_point_array {
  unsigned int index;
  unsigned int size;
  struct camquake_path_point point[1];
};

typedef enum {
  CQP_INTERPOLATE_EQUAL = 1 << 0,
} cqp_flags;

struct camquake_path {
	int flags;
	float time_start, time_stop;
	struct camquake_path_point_array *path;
	struct camquake_path_point *interpolated_array;
	struct camquake_path_point *projected_points;
};

struct camquake_setup {
	char *name;
	struct camquake_path camera_path;
	struct camquake_path view_path;
	float time_start, time_stop;
	struct camquake_setup *next;
	int changed;
};

struct color4f {
	float r, g, b, a;
};

struct camquake_colors {
  struct color4f camera_path, camera_point;
  struct color4f view_path, view_point;
  struct color4f highlight_path, highlight_point;
  struct color4f movement_x, movement_y;
};

struct camquake_color_variables {
	cvar_t camera_path_color, camera_point_color;
	cvar_t view_path_color, view_point_color;
	cvar_t highlight_path_color, highlight_point_color;
	cvar_t movement_x, movement_y;
};

typedef enum {
  CQEM_MA_MIN = 0,
  CQEM_MA_XY = 0,
  CQEM_MA_XZ = 1,
  CQEM_MA_YZ = 2,
  CQEM_MA_VIEW = 3,
  CQEM_MA_X = 4,
  CQEM_MA_Y = 5,
  CQEM_MA_Z = 6,
  CQEM_MA_MAX = 6,
} cqem_movement_axis;

struct camquake_edit_mode {
  int movement_axis;
  float movement_multiplier;
  int help;
  int move_path;
  struct camquake_path_point cursor_old;
  struct camquake_path_point cursor_initial;
};

struct camquake {
	cvar_t enabled;
	cvar_t overwrite_on_load;
	cvar_t render;
	struct camquake_color_variables color_variables;
	struct camquake_colors colors;
	struct camquake_setup *setup;
	struct camquake_setup *active_setup;
	struct camquake_setup *selected_setup;;
	struct camquake_path_point *selected_point;
	struct camquake_path_point *selected_point_p;
	struct camquake_path *selected_path;
	float current_time;
	int have_input;
	struct camquake_edit_mode edit;
	int setup_projection;
};



struct camquake_path_point *CQP_Point_New(struct camquake_path_point_array **path_point_array);
int CQP_Point_Remove_Index(struct camquake_path_point_array **path_point_array, unsigned int position);
struct camquake_path_point *CQP_Point_New_Index(struct camquake_path_point_array **path_point_array, unsigned int index);
void CQP_Print(struct camquake_path_point_array *path_point_array);
void CQP_Free(struct camquake_path *path);
struct camquake_setup *CQS_New(struct camquake_setup **first, char *name);
struct camquake_setup *CQS_Find(struct camquake_setup **first, char *name);
void CQS_Remove(struct camquake_setup **first, char *name);
void Camquake_Cmd(void);
void Camquake_Render_Setup (struct camquake_setup *setup);
void CQS_Interpolate_Path(struct camquake_path_point_array *path, float t, struct camquake_path_point *out);
void CQS_Interpolate(struct camquake_setup *setup, float t, struct camquake_path_point *cam, struct camquake_path_point *view) ;
void Camquake_Vector_To_Angles(struct camquake_path_point *vec, struct camquake_path_point *ang);
void Camquake_InputRelease(void);
void Camquake_InputGrab(void);
void Camquake_Render_Setup_Projected_Points(struct camquake_path *path);
