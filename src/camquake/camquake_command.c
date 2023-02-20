#include "../mathlib.h"
#include "camquake_internal.h"

extern struct camquake *camquake;

void Camquake_Write_Config(cqs_type stype, struct camquake_setup *setup, char *name) {
    char filepath[MAX_PATH];
    FILE *f;
    struct camquake_path_point *p;
    struct camquake_trigger *trigger;
    struct camquake_interpolation *inter;
    int i;
    char *type = NULL;

    if (stype == CQS_SETUP) {
	type = "setup";
    } else if (stype == CQS_UTILITY) {
	type = "utility";
    } else {
	Com_Printf("unkown type\n");
    return;
    }

    snprintf(filepath, sizeof(filepath), "%s/camquake/%s.cfg", com_basedir, name);
    if ((f = fopen(filepath, "w")) != NULL) {
	    fprintf(f, "camquake %s add \"%s\"\n", type, name);

	    if (setup->camera_path.path) {
		for (i=0; i<setup->camera_path.path->index; i++) {
			p = &setup->camera_path.path->point[i];
			fprintf(f, "camquake %s add_camera_point \"%s\" %f %f %f %f\n", type, name, p->x, p->y, p->z, p->time);
		}
	    }
	    if (setup->view_path.path) {
		for (i=0; i<setup->view_path.path->index; i++) {
		    p = &setup->view_path.path->point[i];
		    fprintf(f, "camquake %s add_view_point \"%s\" %f %f %f %f\n",type,  name, p->x, p->y, p->z, p->time);
		}
	    }
	    for (trigger=setup->triggers; trigger != NULL; trigger=trigger->next) {

		    fprintf(f, "camquake %s add_trigger \"%s\" \"%s\" %f \"%s\"\n", type, name, Camquake_Event_Name(trigger->type), trigger->time, trigger->command);
	    }

	    for (inter=setup->interpolations; inter!= NULL; inter=inter->next) {

		    fprintf(f, "camquake %s add_interpolation \"%s\" \"%s\" %f %f \"%s\" %f %f\n", type, name, Camquake_Event_Name(inter->type), inter->time_start, inter->time_stop, inter->command, inter->value_start, inter->value_stop);
	    }
	    if (setup->texture) {
		    fprintf(f, "camquake texture load \"%s\" \"%s\"\n", type, setup->texture->name, setup->texture->texture);
		    fprintf(f, "camquake %s texture \"%s\"\n", type, setup->texture->name);
	    }
	    fclose(f);
	    Com_Printf("%s \'%s\" saved as \"%s\" in \"%s\"\n", type, setup->name, name, filepath);
    } else {
	    Com_Printf("could not save to \"%s\"\n", filepath);
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
	Com_Printf("add_view_point [name] [current|x y z t] -- adds a point to the view path. \"current\" sets the point to 50px infront of you and the time, \"x y z t\" sets it to absolute coordinates and time\n");
	Com_Printf("add_camera_point [name] [current|x y z] -- adds a point to the camera path. \"current\" sets the point your current location and time, \"x y z t\" sets it to absolute coordinates and time\n");
	Com_Printf("list -- list available setups\n");
}

void Camquake_Print_Setup(struct camquake_setup *setup) {
	int i;
	struct camquake_trigger *trigger;
	struct camquake_interpolation *inter;
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
		Com_Printf("      %f %f %f - %f\n", p->x, p->y, p->z, p->time);
	    }
	} else { 
	    Com_Printf("camera: no points.\n");
	}
	if (setup->view_path.path) {
	    Com_Printf("view: %d points.\n", setup->view_path.path->index);
	    for (i=0; i<setup->view_path.path->index; i++) {
		    p = &setup->view_path.path->point[i];
		    Com_Printf("      %f %f %f - %f\n", p->x, p->y, p->z, p->time);
	    }
	} else {
	    Com_Printf("view: no points.\n");
	}

	trigger = setup->triggers;
	if (trigger) {
	    i = 0;
	    Com_Printf("Triggers: \n");
	    while(trigger) {
		i++;
		Com_Printf("%i: %s %f \"%s\"\n", i, Camquake_Event_Name(trigger->type),  trigger->time, trigger->command);
		trigger = trigger->next;
	    }
	} else {
	    Com_Printf("No triggers.\n");
	}

	inter = setup->interpolations;
	if (inter) {
	    i = 0;
	    Com_Printf("Interpolations: \n");
	    while(inter) {
		i++;
		Com_Printf("%i: %s %f %f \"%s\" %f %f\n", i, Camquake_Event_Name(inter->type),  inter->time_start, inter->time_stop, inter->command, inter->value_start, inter->value_stop);
		inter = inter->next;
	    }
	} else {
	    Com_Printf("No interpolations.\n");
	}
}

void Camquake_Edit(void) {
    Camquake_InputGrab();
    camquake->have_input = 1;
}

void Camquake_Setup(cqs_type stype) {
	struct camquake_setup *setup;
	struct camquake_path_point point;
	struct camquake_setup **setup_type;
	char *type;
	int i;

	if (stype == CQS_UTILITY) {
	    type = "utility";
	    setup_type = &camquake->utility;
	} else {
	    type = "setup";
	    setup_type = &camquake->setup;
	}

	if (strcmp(Cmd_Argv(2), "add") == 0) {
		if (Cmd_Argc() <= 3) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_New(setup_type, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("Could not add a %s with the name %s\n", type, Cmd_Argv(3));
			return;
		}
		Com_Printf("%s \"%s\" added.\n", type, Cmd_Argv(3));
	} else if (strcmp(Cmd_Argv(2), "remove") == 0) {
		if (Cmd_Argc() != 4) {
			Com_Printf("camquake remove \"%s name\"\n", type);
			return;
		}
		setup = CQS_Find(setup_type, Cmd_Argv(3));
		if (setup == NULL) {
		    Com_Printf("%s \"%s\" not found.\n", type, Cmd_Argv(3));
		    return;
		} else {
		    if (camquake->selected_setup == setup) {
			camquake->selected_setup = NULL;
			camquake->selected_path = NULL;
			camquake->selected_point = NULL;
		    }
		    CQS_Remove(setup_type, Cmd_Argv(3));
		    Com_Printf("%s \"%s\" removed.\n", type, Cmd_Argv(3));
		}
	} else if (strcmp(Cmd_Argv(2), "list") == 0) {
		if (camquake->setup == NULL) {
			Com_Printf("no setups available.\n");
			return;
		}
		setup = *setup_type;
		while (setup != NULL) {
			Camquake_Print_Setup(setup);
			setup = setup->next;
		}

	} else if (strcmp(Cmd_Argv(2), "add_interpolation") == 0) {
	    Camquake_Add_Interpolation();
	    return;
	} else if (strcmp(Cmd_Argv(2), "add_trigger") == 0) {
	    Camquake_Add_Trigger();
	    return;
	} else if (strcmp(Cmd_Argv(2), "add_camera_point") == 0) {
		if (Cmd_Argc() < 5 || Cmd_Argc() > 8) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(setup_type, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("%s \"%s\" not found\n", type, Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "current") == 0) {
			point.x = r_refdef.vieworg[0];
			point.y = r_refdef.vieworg[1];
			point.z = r_refdef.vieworg[2];

			if (strcmp(Cmd_Argv(5), "now") == 0) {
			    point.time = cls.demotime;
			} else {
			    point.time = atof(Cmd_Argv(5));
			}
			if (setup->time_start == 0) {
			    setup->time_start= point.time;
			}
			if (setup->time_stop <= point.time) {
			    setup->time_stop = point.time;
			}
			CQP_Insert_Point(&setup->camera_path.path, &point);
			setup->changed = 1;
			return;
		} else {
			if (Cmd_Argc() != 8) {
				Camquake_Help_Setup();
				return;
			}
			point.x = atof(Cmd_Argv(4));
			point.y = atof(Cmd_Argv(5));
			point.z = atof(Cmd_Argv(6));
			point.time = atof(Cmd_Argv(7));
			if (setup->time_start == 0) {
			    setup->time_start= point.time;
			}
			if (setup->time_stop <= point.time) {
			    setup->time_stop = point.time;
			}
			CQP_Insert_Point(&setup->camera_path.path, &point);
			setup->changed = 1;
			return;
		}
	} else if (strcmp(Cmd_Argv(2), "add_view_point") == 0) {
		if (Cmd_Argc() < 5 || Cmd_Argc() > 8) {
			Camquake_Help_Setup();
			return;
		}
		setup = CQS_Find(setup_type, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("%s \"%s\" not found\n", type, Cmd_Argv(3));
			return;
		}
		if (strcmp(Cmd_Argv(4), "current") == 0) {
			vec3_t forward, up, right;
			AngleVectors(r_refdef.viewangles, forward, right, up);
			point.x = forward[0] * 150 + r_refdef.vieworg[0];
			point.y = forward[1] * 150 + r_refdef.vieworg[1];
			point.z = forward[2] * 150 + r_refdef.vieworg[2];

			if (strcmp(Cmd_Argv(5), "now") == 0) {
			    point.time = cls.demotime;
			} else {
			    point.time = atof(Cmd_Argv(5));
			}
			if (setup->time_start == 0) {
			    setup->time_start= point.time;
			}
			if (setup->time_stop <= point.time) {
			    setup->time_stop = point.time;
			}
			CQP_Insert_Point(&setup->view_path.path, &point);
			setup->changed = 1;
			return;
		} else {
			if (Cmd_Argc() != 8) {
				Camquake_Help_Setup();
				return;
			}
			point.x = atof(Cmd_Argv(4));
			point.y = atof(Cmd_Argv(5));
			point.z = atof(Cmd_Argv(6));
			point.time = atof(Cmd_Argv(7));
			if (setup->time_start == 0) {
			    setup->time_start= point.time;
			}
			if (setup->time_stop <= point.time) {
			    setup->time_stop = point.time;
			}
			CQP_Insert_Point(&setup->view_path.path, &point);
			setup->changed = 1;
			return;
		}
	} else if (strcmp(Cmd_Argv(2), "texture") == 0) {
	    if (stype != CQS_UTILITY) {
		Com_Printf("can only assign a texture to a utility\n");
		return;
	    }
	    setup = CQS_Find(setup_type, Cmd_Argv(3));
	    if (setup == NULL) {
		Com_Printf("%s \"%s\" not found\n", type, Cmd_Argv(3));
		return;
	    }
	    struct camquake_texture *t;
	    t = Camquake_Texture_Find(Cmd_Argv(4), NULL);
	    if (t == NULL) {
		Com_Printf("could not find texture \"%s\"", Cmd_Argv(4));
		return;
	    }
	    setup->texture = t;
	    return;
	} else if (strcmp(Cmd_Argv(2), "write") == 0) {
		setup = CQS_Find(setup_type, Cmd_Argv(3));
		if (setup == NULL) {
			Com_Printf("%s \"%s\" not found\n", type, Cmd_Argv(3));
			return;
		}
		if (Cmd_Argc() == 5) {
			Camquake_Write_Config(stype, setup, Cmd_Argv(4));
		} else {
			Camquake_Write_Config(stype, setup, setup->name);
		}
	}
}

void Camquake_Playback(void) {
	struct camquake_setup *setup;
	struct camquake_setup **setup_type, **active_type;
	if (Cmd_Argc() < 4) {
	    Com_Printf("setup|itility start|stop name.\n");
	    return;
	}
	if (strcmp(Cmd_Argv(2), "setup") == 0) {
	    setup_type = &camquake->setup;
	    active_type = &camquake->active_setup;
	} else if (strcmp(Cmd_Argv(2), "utility") == 0) {
	    setup_type = &camquake->utility;
	    active_type = &camquake->active_utility;
	} else {
	    Com_Printf("setup|itility start|stop name.\n");
	    return;
	}
	if (strcmp(Cmd_Argv(3), "start") == 0) {
	    if (Cmd_Argc() < 4) {
		    Com_Printf("need a camera setup to play.\n");
		    return;
	    }
	    setup = CQS_Find(setup_type, Cmd_Argv(4));
	    if (setup == NULL) {
		Com_Printf("\"%s\" does not exist. try camquake setup list\n");
		return;
	    }
	    *active_type  = setup;

	    if (Cmd_Argc() == 5) {
		camquake->current_time = atof(Cmd_Argv(4));
	    } else {
		camquake->current_time = 0;
	    }
	} else if (strcmp(Cmd_Argv(2), "stop") == 0) {
	    camquake->current_time = 0;
	    if (*active_type) {
		(*active_type)->first_frame = 0;
		*active_type = NULL;
	    }
	} else {
	    Com_Printf("setup|utility start|stop.\n");
	}
	return;
}


void Camquake_Select(void) {
	struct camquake_setup *setup;
	struct camquake_setup **setup_type, **selected_type;
	char *type;
	if (Cmd_Argc() < 4) {
		Com_Printf("select setup|utility name.\n");
		return;
	}
	if (strcmp(Cmd_Argv(2), "setup") == 0) {
	    setup_type = &camquake->setup;
	    type = "setup";
	} else if (strcmp(Cmd_Argv(2), "utility") == 0) {
	    setup_type = &camquake->utility;
	    type = "utility";
	} else {
		Com_Printf("select setup|utility name.\n");
	    return;
	}
	setup = CQS_Find(setup_type, Cmd_Argv(3));
	if (setup == NULL) {
		Com_Printf("%s \"%s\" does not exist. try camquake %s list\n", type, type);
		return;
	}
	camquake->selected_setup = setup;
	Com_Printf("%s \"%s\" selected.\n", type, setup->name);
}


void Camquake_Cmd(void) {
	int i;
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
	} else if (strcmp(Cmd_Argv(1), "utility") == 0) {
		Camquake_Setup(CQS_UTILITY);
		return;
	} else if (strcmp(Cmd_Argv(1), "setup") == 0) {
		Camquake_Setup(CQS_SETUP);
		return;
	} else if (strcmp(Cmd_Argv(1), "texture") == 0) {
		Camquake_Texture_Command(2, Cmd_Argc() - 2);
		return;
	} else if (strcmp(Cmd_Argv(1), "playback") == 0) {
		Camquake_Playback();
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

