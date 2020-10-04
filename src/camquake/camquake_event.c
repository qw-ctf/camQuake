#include "../common.h"
#include "../mathlib.h"
#include "camquake_internal.h"

extern struct camquake *camquake;

void Camquake_Event_Print_Types(void) {
	Com_Printf("available event types: console, camera_roll\n");
}

cqe_type Camquake_Event_Type(char *event) {
	if (strcmp(event, "console") == 0) {
		return CQE_CONSOLE;
	} else if (strcmp(event, "camera_roll") == 0) {
		return CQE_CAMERA_ROLL;
	}
	return CQE_ERROR;
}

char *Camquake_Event_Name(cqe_type event) {
	switch (event) {
		case CQE_CONSOLE:
			return "console";
		case CQE_CAMERA_ROLL:
			return "camera_roll";
		case CQE_ERROR:
			return "ERROR";
	}
	return "NOT A VALID EVENT TYPE";
}

void Camquake_Add_Trigger(void) {
    struct camquake_setup *setup;
    struct camquake_trigger *trigger, *ct;
    setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
    if (setup == NULL) {
	Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
	return;
    }

    trigger = calloc(1, sizeof(*trigger));
    if (trigger == NULL)
	    return;
    trigger->type = Camquake_Event_Type(Cmd_Argv(4));
    if (trigger->type == CQE_ERROR) {
	    Com_Printf("%s is not a valid event type.\n");
	    Camquake_Event_Print_Types();
	    free(trigger);
	    return;
    }
    if (strcmp(Cmd_Argv(5), "now") == 0) {
	    trigger->time = cls.demo_rewindtime;
    } else {
	    trigger->time = atof(Cmd_Argv(5));
    }
    trigger->command = strdup(Cmd_Argv(6));
    if (setup->triggers == NULL) {
	    setup->triggers = trigger;
    } else {
	    ct = setup->triggers;
	    while(ct) {
		    if (ct->next == NULL) {
			    ct->next = trigger;
		    }
		    ct = ct->next;
	    }
    }
    Com_Printf("trigger added.\n");
}

void Camquake_Triggers(struct camquake_setup *setup, float t) {
	struct camquake_trigger *trigger;
	trigger = setup->triggers;
	while (trigger) {
		if (trigger->time - setup->time_start <= t && trigger->frame != setup->first_frame) {
			Cbuf_AddText(trigger->command);
			trigger->frame = setup->first_frame;
		}
		trigger = trigger->next;
	}
}

void Camquake_Add_Interpolation(void) {
    struct camquake_setup *setup;
    struct camquake_interpolation *interpolation, *ci;
    setup = CQS_Find(&camquake->setup, Cmd_Argv(3));
    if (setup == NULL) {
	Com_Printf("setup \"%s\" not found\n", Cmd_Argv(3));
	return;
    }

    interpolation = calloc(1, sizeof(*interpolation));
    if (interpolation == NULL)
	    return;

    interpolation->type = Camquake_Event_Type(Cmd_Argv(4));
    if (interpolation->type == CQE_ERROR) {
	    Com_Printf("%s is not a valid event type.\n");
	    Camquake_Event_Print_Types();
	    free(interpolation);
	    return;
    }
    interpolation->time_start = atof(Cmd_Argv(5));
    interpolation->time_stop = atof(Cmd_Argv(6));
    interpolation->command = strdup(Cmd_Argv(7));
    interpolation->value_start = atof(Cmd_Argv(8));
    interpolation->value_stop = atof(Cmd_Argv(9));

    if (setup->interpolations == NULL) {
	    setup->interpolations = interpolation;
    } else {
	    ci = setup->interpolations;
	    while (ci) {
		    if (ci->next == NULL) {
			    ci->next = interpolation;
		    }
		    ci = ci->next;
	    }
    }

    Com_Printf("interpolation added.\n");
}

void Camquake_Interpolations(struct camquake_setup *setup, float t) {
	struct camquake_interpolation *inter;
	char buffer[1024 * 4];
	float dt, ot, v, st, et;
	inter = setup->interpolations;
	while (inter) {
		st = inter->time_start - setup->time_start;
		et = inter->time_stop- setup->time_start;
		if (st <= t && et >= t) {
			ot = et - st;
			dt = t - st;
			v = inter->value_start;
			v += (inter->value_stop - inter->value_start) * (dt/ot) ;
			snprintf(buffer, sizeof(buffer), "%s %f", inter->command, v);
			Cbuf_AddText(buffer);
		}
		inter = inter->next;
	}
}
