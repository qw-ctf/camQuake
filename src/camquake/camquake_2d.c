#include "../quakedef.h"
#include "../keys.h"
#include "camquake_internal.h"

extern struct camquake *camquake;

char *movement_types[CQEM_MA_MAX + 1] = { "xy", "xz", "yz", "view", "x", "y", "z"};

void draw_on_screen(int *line, char *fmt, ...) {
	va_list argptr;
	char msg[1024];
	int line_pixel = 8; // might be bad idea
	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);
	Draw_ColoredString(0, *line * line_pixel, msg, 0, false);
	*line += 1;
}

char *Camquake_Get_Select_Mode(cqem_select_mode mode) {
	switch (mode) {
		case CQEM_SELECT_MODE_PATH:
			return "&cf00path&cfff";
		case CQEM_SELECT_MODE_POINT:
			return "&c0f0point&cfff";
		case CQEM_SELECT_MODE_SEGMENT:
			return "&c00fsegment&cfff";
	}
}

void Camquake_2D_Draw(void)
{
	int line = 0;
	int *lptr = &line;


	if (camquake->have_input < 1) {
		return;
	}

	draw_on_screen(lptr, "camquake edit (%s) mode (press f1 %s help/escape to exit)", (camquake->edit.edit_mode == 0) ? "&c0f0position&cfff" : "&cf00time&cfff", camquake->edit.help ? "to close" : "for");
	if (camquake->edit.edit_mode == 0) {
		draw_on_screen(lptr, "movement: direction(%s) mode(%s) modifier(%f)", movement_types[camquake->edit.movement_axis], Camquake_Get_Select_Mode(camquake->edit.select_mode), camquake->edit.movement_multiplier);
	} else {
		draw_on_screen(lptr, "time: modifier(%f)",  camquake->edit.movement_multiplier);
	}

	if (camquake->edit.help == 1) {

		draw_on_screen(lptr, "hover over a point to select it, it will turn camquake_color_highlight_point");
		draw_on_screen(lptr, "keep mouse1 pressed to move it");
		draw_on_screen(lptr, "     the lines colored (camquake_color_movement_x/camquake_color_movement_y)");
		draw_on_screen(lptr, "     will show the direction the point will move when you move the mouse");
		draw_on_screen(lptr, "press 1|2 to cycle through the movement modes - %s", movement_types[camquake->edit.movement_axis]);
		draw_on_screen(lptr, "press 3 to cycle selecting a single point, path or segment");
		draw_on_screen(lptr, "press 4|5 to divide/multiply movement multiplier by 10 - %f", camquake->edit.movement_multiplier);
		draw_on_screen(lptr, "press tab to switch between &c0f0position&cfff and &cf00time&cfff editing");
		return;
	}

	if (camquake->selected_point)
		draw_on_screen(lptr, "selected point: %f %f %f - %f", camquake->selected_point->x, camquake->selected_point->y, camquake->selected_point->z, camquake->selected_point->time);
}
