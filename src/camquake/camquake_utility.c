#include "../mathlib.h"
#include "camquake_internal.h"

extern struct camquake *camquake;


void Camquake_Utility_Render_Frame(void) {
	struct camquake_setup *current;
	float t;
	// if there is an active utility, only this one will be played
	if (camquake->active_utility) {
		current = camquake->active_utility;
		if (camquake->current_time <= current->time_stop - current->time_start) {
			if (current->first_frame == 0) {
				current->first_frame = camquake->frame;
			}
			Camquake_Render_Texture(current, camquake->current_time);
		} else {
			camquake->active_utility->first_frame = 0;
			camquake->active_utility = NULL;
		}

	} else {
		if (camquake->auto_playback.value == 1) {
			for (current = camquake->utility; current != NULL; current = current->next) {
				if (cls.demotime >= current->time_start && cls.demotime <= current->time_stop) {
					if (current->first_frame == 0) {
						current->first_frame = camquake->frame;
					}
					t = (cls.demotime - current->time_start);
					Camquake_Render_Texture(current, t);
				} else {
					current->first_frame = 0;
				}
			}
		}
	}
}
