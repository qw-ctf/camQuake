#include "../common.h"
#include "../mathlib.h"
#include "camquake_internal.h"

extern struct camquake *camquake;

void Camquake_Write_Config(struct camquake_setup *setup, char *name) {
    char filepath[MAX_PATH];
    FILE *f;
    struct camquake_path_point *p;
    int i;

    snprintf(filepath, sizeof(filepath), "%s/camquake/%s.cfg", com_basedir, name);
    if ((f = fopen(filepath, "w")) != NULL) {
	    fprintf(f, "camquake setup add \"%s\"\n", name);
	    fprintf(f, "camquake setup start_time \"%s\" %f\n", name, setup->time_start);
	    fprintf(f, "camquake setup stop_time \"%s\" %f\n", name, setup->time_stop);

	    for (i=0; i<setup->camera_path.path->index; i++) {
		    p = &setup->camera_path.path->point[i];
		    fprintf(f, "camquake setup add_camera_point \"%s\" %f %f %f\n", name, p->x, p->y, p->z);
	    }
	    for (i=0; i<setup->view_path.path->index; i++) {
		    p = &setup->view_path.path->point[i];
		    fprintf(f, "camquake setup add_view_point \"%s\" %f %f %f\n", name, p->x, p->y, p->z);
	    }
	    fclose(f);
	    Com_Printf("setup \'%s\" saved as \"%s\" in \"%s\"", setup->name, name, filepath);
    } else {
	    Com_Printf("could not save to \"%s\".", filepath);
    }
}

void Camquake_Available_Commands(void) {
	Com_Printf("Available commands:\n");
	Com_Printf("help\n");
	Com_Printf("setup\n");
	Com_Printf("play\n");
	Com_Printf("select\n");
	Com_Printf("edit\n");
	Com_Printf("write\n");
}

void Camquake_Help(void) {
	Com_Printf("try: camquake help command\n");
	Camquake_Available_Commands();
}

void Camquake_Help_Edit(void) {
	Com_Printf("enter edit mode\n");
}


void Camquake_Help_Write(void) {
	Com_Printf("save a setup to a config\n");
	Com_Printf("camquake write my_cam -- will write my_cam to camquake/my_cam.cfg");
	Com_Printf("camquake write my_cam new_name -- will write my_cam to camquake/new_name.cfg");
}

void Camquake_Help_Select(void) {
	Com_Printf("select a setup for rendering/editing\n");
	Com_Printf("camquake select my_cam");
}

void Camquake_Help_Play(void) {
	Com_Printf("start playing a setup\n");
	Com_Printf("camquake play my_cam");
}

void Camquake_Help_Setup(void) {
	Com_Printf("available setup commands:\n");
	Com_Printf("add [name] -- adds a new setup\n");
	Com_Printf("remove [name] -- removes the setup\n");
	Com_Printf("start_time [name] [now|x.x] -- sets start time to current demotime with \"now\" or some arbitary float value\n");
	Com_Printf("stop_time [name] [now|x.x] -- sets stop time to current demotime with \"now\" or some arbitary float value\n");
	Com_Printf("add_view_point [name] [current|x y z] -- adds a point to the view path. \"current\" sets the point to 50px infront of you, \"x y z\" sets it to absolute coordinates \n");
	Com_Printf("add_camera_point [name] [current|x y z] -- adds a point to the camera path. \"current\" sets the point your current location, \"x y z\" sets it to absolute coordinates \n");
	Com_Printf("list -- list available setups\n");
}

void Camquake_Print_Setup(struct camquake_setup *setup) {
	int i;
	if (setup == NULL) {
		return;
	}
	struct camquake_path_point *p;
	Com_Printf("%s:\n", setup->name);
	Com_Printf("time: %f - %f\n", setup->time_start, setup->time_stop);
	if (setup->camera_path.path) {
	    Com_Printf("camera: %d points.\n", setup->camera_path.path->index);
	    for (i=0; i<setup->camera_path.path->index; i++) {
		p = &setup->camera_path.path->point[i];
		Com_Printf("      %f %f %f\n", p->x, p->y, p->z);
	    }
	} else { 
	    Com_Printf("camera: no points.\n");
	}
	if (setup->view_path.path) {
	    Com_Printf("view: %d points.\n", setup->view_path.path->index);
	    for (i=0; i<setup->view_path.path->index; i++) {
		    p = &setup->view_path.path->point[i];
		    Com_Printf("      %f %f %f\n", p->x, p->y, p->z);
	    }
	} else {
	    Com_Printf("view: no points.\n");
	}
}

void Camquake_Edit(void) {
    Camquake_InputGrab();
    camquake->have_input = 1;
}

void Camquake_Setup(void) {
	struct camquake_setup *setup;
	struct camquake_path_point *point;
	if (strcmp(Cmd_Argv(2), "add") == 0) {
		if (Cmd_Argc() <= 3) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_New(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("Could not add a setup with the name %s\n", Cmd_Argv(3));
			return;
		}
		Com_Printf("setup \"%s\" added.\n", Cmd_Argv(3));
	} else if (strcmp(Cmd_Argv(2), "remove") == 0) {
		if (Cmd_Argc() != 4) {
			Com_Printf("camquake remove \"setup name\"\n");
			return;
		}
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
		    Com_Printf("setup \"%s\" not found.\n", Cmd_Argv(3));
		    return;
		} else {
		    if (camquake->selected_setup == setup) {
			camquake->selected_setup = NULL;
			camquake->selected_path = NULL;
			camquake->selected_point = NULL;
		    }
		    CQS_Remove(&camquake->setup, Cmd_Argv(3));
		    Com_Printf("setup \"%s\" removed.\n", Cmd_Argv(3));
		}
	} else if (strcmp(Cmd_Argv(2), "list") == 0) {
		if (camquake->setup == NULL) {
			Com_Printf("no setups available.\n");
			return;
		}
		setup = camquake->setup;
		while (setup != NULL) {
			Camquake_Print_Setup(setup);
			setup = setup->next;
		}

	} else if (strcmp(Cmd_Argv(2), "start_time") == 0) {
		if (Cmd_Argc() != 5) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "now") == 0) {
			setup->time_start = cls.demotime;
		} else {
			setup->time_start = atof(Cmd_Argv(4));
		}
		setup->changed = 1;
		return;
	} else if (strcmp(Cmd_Argv(2), "stop_time") == 0) {
		if (Cmd_Argc() != 5) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "now") == 0) {
			setup->time_stop = cls.demotime;
		} else {
			setup->time_stop = atof(Cmd_Argv(4));
		}
		setup->changed = 1;
		return;
	} else if (strcmp(Cmd_Argv(2), "add_camera_point") == 0) {
		if (Cmd_Argc() < 5 || Cmd_Argc() > 7) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "current") == 0) {
			point = CQP_Point_New(&(setup->camera_path.path));
			if (point == NULL) {
				Com_Printf("Could not add point.\n");
				return;
			}
			point->x = r_refdef.vieworg[0];
			point->y = r_refdef.vieworg[1];
			point->z = r_refdef.vieworg[2];
			setup->changed = 1;
			return;
		} else {
			if (Cmd_Argc() != 7) {
				Camquake_Help_Setup();
				return;
			}
			point = CQP_Point_New(&(setup->camera_path.path));
			if (point == NULL) {
				Com_Printf("Could not add point.\n");
				return;
			}
			point->x = atof(Cmd_Argv(4));
			point->y = atof(Cmd_Argv(5));
			point->z = atof(Cmd_Argv(6));
			setup->changed = 1;
			return;
		}
	} else if (strcmp(Cmd_Argv(2), "add_view_point") == 0) {
		if (Cmd_Argc() < 5 || Cmd_Argc() > 7) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "current") == 0) {
			point = CQP_Point_New(&(setup->view_path.path));
			if (point == NULL) {
				Com_Printf("Could not add point.\n");
				return;
			}

			vec3_t forward, up, right;
			AngleVectors(r_refdef.viewangles, forward, right, up);
			point->x = forward[0] * 150 + r_refdef.vieworg[0];
			point->y = forward[1] * 150 + r_refdef.vieworg[1];
			point->z = forward[2] * 150 + r_refdef.vieworg[2];
			setup->changed = 1;
			return;
		} else {
			if (Cmd_Argc() != 7) {
				Camquake_Help_Setup();
				return;
			}
			point = CQP_Point_New(&(setup->view_path.path));
			if (point == NULL) {
				Com_Printf("Could not add point.\n");
				return;
			}
			point->x = atof(Cmd_Argv(4));
			point->y = atof(Cmd_Argv(5));
			point->z = atof(Cmd_Argv(6));
			setup->changed = 1;
			return;
		}
	} else if (strcmp(Cmd_Argv(2), "write") == 0) {
		setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
			return;
		}
		if (Cmd_Argc() == 5) {
			Camquake_Write_Config(setup, Cmd_Argv(4));
		} else {
			Camquake_Write_Config(setup, setup->name);
		}
	}
}


void Camquake_Play(void) {
	struct camquake_setup *setup;
	if (Cmd_Argc() < 3) {
		Com_Printf("need a camera setup to play.\n");
		return;
	}
	setup = CQS_Find(&camquake->setup, Cmd_Argv(2));
	if (setup == NULL) {
		Com_Printf("setup \"%s\" does not exist. try camquake setup list\n");
		return;
	}
	camquake->active_setup = setup;
	camquake->current_time = 0;
}


void Camquake_Select(void) {
	struct camquake_setup *setup;
	if (Cmd_Argc() < 3) {
		Com_Printf("need a camera setup to select.\n");
		return;
	}
	setup = CQS_Find(&camquake->setup, Cmd_Argv(2));
	if (setup == NULL) {
		Com_Printf("setup \"%s\" does not exist. try camquake setup list\n");
		return;
	}
	camquake->selected_setup = setup;
	Com_Printf("setup \"%s\" selected.\n");
}


void Camquake_Cmd(void) {
	if (Cmd_Argc() < 2) {
		Camquake_Available_Commands();
		return;
	}
	if (strcmp(Cmd_Argv(1), "help") == 0) {
		if (Cmd_Argc() < 3) {
			Camquake_Help();
			return;
		}
		if (strcmp(Cmd_Argv(2), "setup") == 0) {
			Camquake_Help_Setup();
			return;
		}
		if (strcmp(Cmd_Argv(2), "edit") == 0) {
			Camquake_Help_Edit();
			return;
		}
		if (strcmp(Cmd_Argv(2), "write") == 0) {
			Camquake_Help_Write();
			return;
		}
		if (strcmp(Cmd_Argv(2), "select") == 0) {
			Camquake_Help_Select();
			return;
		}
		if (strcmp(Cmd_Argv(2), "play") == 0) {
			Camquake_Help_Play();
			return;
		}
		Camquake_Help();
		return;
	} else if (strcmp(Cmd_Argv(1), "setup") == 0) {
		Camquake_Setup();
		return;
	} else if (strcmp(Cmd_Argv(1), "play") == 0) {
		Camquake_Play();
		return;
	} else if (strcmp(Cmd_Argv(1), "select") == 0) {
		Camquake_Select();
		return;
	} else if (strcmp(Cmd_Argv(1), "edit") == 0) {
		Camquake_Edit();
		return;
	}
	Camquake_Available_Commands();
}
