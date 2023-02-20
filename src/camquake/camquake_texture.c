#include "camquake_internal.h"

extern struct camquake *camquake;

int Camquake_Texture_Load(char *name, char *texture) {
	struct camquake_texture *t, *ct, *prev;

	t = Camquake_Texture_Find(name, &prev);
	if (t != NULL) {
		return(CQ_EXISTS);
	}

	t = calloc(1, sizeof(struct camquake_texture));
	if (t == NULL) {
		return CQ_ALLOCATE;
	}
	t->name = strdup(name);
	if (t->name == NULL) {
		free(t);
		return CQ_ALLOCATE;
	}
	t->texture = strdup(texture);
	if (t->texture== NULL) {
		free(t->name);
		free(t);
		return CQ_ALLOCATE;
	}
	t->texture_ref = R_LoadTextureImage(texture, name, 0, 0, TEX_ALPHA);
	if (prev != NULL) {
		prev->next = t;
		return CQ_OK;
	}
	if (camquake->textures == NULL) {
		camquake->textures = t;
		return CQ_OK;
	}
	return CQ_WTF;
}

int Camquake_Texture_Delete(char *name) {
	struct camquake_texture *t, *prev;
	t = Camquake_Texture_Find(name, &prev);
	if (t) {
		if (prev) {
			prev->next = t->next;
		} else {
			camquake->textures = t->next;
		}
		return Camquake_Texture_Free(t);
	}
	return CQ_NOTFOUND;
}

int Camquake_Texture_Free(struct camquake_texture *t) {
	R_DeleteTexture(&t->texture_ref);
	free(t->name);
	free(t->texture);
	free(t);
	return CQ_OK;
}

struct camquake_texture *Camquake_Texture_Find(char *name, struct camquake_texture **prev) {
	struct camquake_texture *t;
	if (prev) {
		*prev = (struct camquake_texture *)NULL;
	}
	for (t = camquake->textures; t!= NULL; t = t->next) {
		if (strcmp(name, t->name) == 0) {
			return t;
		}
		prev = NULL;
	}
	return NULL;
}
