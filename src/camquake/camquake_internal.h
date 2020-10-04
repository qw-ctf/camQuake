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
	unsigned int first_frame;
	struct camquake_trigger *triggers;
	struct camquake_interpolation *interpolations;
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


typedef enum {
  CQEM_SELECT_MODE_MIN= 0,
  CQEM_SELECT_MODE_POINT = 0,
  CQEM_SELECT_MODE_PATH = 1,
  CQEM_SELECT_MODE_SEGMENT = 2,
  CQEM_SELECT_MODE_MAX= 2,
} cqem_select_mode;

struct camquake_edit_mode {
  int edit_mode;
  int movement_axis;
  float movement_multiplier;
  int help;
  cqem_select_mode select_mode;
  struct camquake_path_point cursor_old;
  struct camquake_path_point cursor_initial;
};

struct camquake_segment {
  int valid;
  struct camquake_path_point *p0, *p1;
};

struct camquake {
	cvar_t enabled;
	cvar_t auto_playback;
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
	struct camquake_segment selected_segment;
	float current_time;
	int have_input;
	struct camquake_edit_mode edit;
	int setup_projection;
	unsigned int frame;
};

typedef enum {
  CQE_CONSOLE = 0,
  CQE_CAMERA_ROLL = 1,
  CQE_ERROR = 999,
} cqe_type ;

struct camquake_trigger {
  struct camquake_trigger *next;
  cqe_type type;
  float time;
  char *command;
  unsigned int frame;
};

struct camquake_interpolation {
  struct camquake_interpolation *next;
  cqe_type type;
  float time_start, time_stop;
  float value_start, value_stop;
  char *command;
  unsigned int frame;
};




struct camquake_path_point *CQP_Point_New(struct camquake_path_point_array **path_point_array);
int CQP_Point_Remove_Index(struct camquake_path_point_array **path_point_array, unsigned int position);
struct camquake_path_point *CQP_Point_New_Index(struct camquake_path_point_array **path_point_array, unsigned int index);
void CQP_Insert_Point(struct camquake_path_point_array **path_point_array, struct camquake_path_point *p);
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
char *Camquake_Event_Name(cqe_type event);
void Camquake_Interpolations(struct camquake_setup *setup, float t);
void Camquake_Triggers(struct camquake_setup *setup, float t);
