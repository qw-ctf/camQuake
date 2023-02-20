#include "camquake_internal.h"

extern struct camquake *camquake;

void Camquake_Texture_Command_Help(void) {
	Com_Printf("Usage: camquake textre command\n");
	Com_Printf("Available commands:\n");
	Com_Printf("    load - loads a texture. Usage: load \"name\" \"texture\"\n");
}

// camquake texture - 2, 0
// camquake texture load bla - 2, 2
void Camquake_Texture_Command(int offset, int count) {
	int error;
	struct camquake_texture *t;
	if (count == 0) {
		Camquake_Texture_Command_Help();
		return;
	}
	count -= 1;
	if (strcmp(Cmd_Argv(offset), "load") == 0) {
		if (count != 2) {
			Camquake_Texture_Command_Help();
			return;
		}
		error = Camquake_Texture_Load(Cmd_Argv(offset + 1), Cmd_Argv(offset+2));
		if (error == CQ_OK) {
			Com_Printf("texture \%s\" loaded as \"%s\"\n", Cmd_Argv(offset+2), Cmd_Argv(offset+1));
			return;
		}
		Com_Printf("texture loading error %i\n", error);
		return;
	} else if (strcmp(Cmd_Argv(offset), "list") == 0) {
		t = camquake->textures;
		while (t) {
			Com_Printf("%s - %s\n", t->name, t->texture);
			t = t->next;
		}
	}

}
