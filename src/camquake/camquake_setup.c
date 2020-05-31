#include "../quakedef.h"
#include "camquake_internal.h"

void Interpolate_Linear(struct camquake_path_point *p0, struct camquake_path_point *p1, float t, struct camquake_path_point *out) {
	out->x = p0->x + (p1->x - p0->x) * t;
	out->y = p0->y + (p1->y - p0->y) * t;
	out->z = p0->z + (p1->z - p0->z) * t;
}

void Interpolate_Bspline(struct camquake_path_point *p0, struct camquake_path_point *p1, struct camquake_path_point *p2, struct camquake_path_point *p3, float delta, struct camquake_path_point *out) {
	float t1, t2, t3, t4;
	t1 = p0->x;
	t2 = -0.5 * p0->x + 0.5 * p1->x;
	t3 = p0->x + -2.5 * p1->x + 2.0 * p2->x + - 0.5 * p3->x;
	t4 = -0.5 * p0->x + 1.5 * p1->x - 1.5 * p2->x + 0.5 * p3->x;
	out->x = ((t4 * delta + t3) * delta + t2) * delta + t1;
	t1 = p0->y;
	t2 = -0.5 * p0->y + 0.5 * p1->y;
	t3 = p0->y + -2.5 * p1->y + 2.0 * p2->y + - 0.5 * p3->y;
	t4 = -0.5 * p0->y + 1.5 * p1->y - 1.5 * p2->y + 0.5 * p3->y;
	out->y = ((t4 * delta + t3) * delta + t2) * delta + t1;
	t1 = p0->z;
	t2 = -0.5 * p0->z + 0.5 * p1->z;
	t3 = p0->z + -2.5 * p1->z + 2.0 * p2->z + - 0.5 * p3->z;
	t4 = -0.5 * p0->z + 1.5 * p1->z - 1.5 * p2->z + 0.5 * p3->z;
	out->z = ((t4 * delta + t3) * delta + t2) * delta + t1;
	return;
}


void Interpolate_Bezier(struct camquake_path_point *p0, struct camquake_path_point *p1, struct camquake_path_point *p2, float delta, struct camquake_path_point *out) {
	float t1, t2, t3;
	t1 = pow(1 - delta, 2) * p0->x;
	t2 = 2 * (1 - delta) * delta * p1->x;
	t3 = pow(delta, 2) * p2->x;
	out->x = t1 + t2 + t3;
	t1 = pow(1 - delta, 2) * p0->y;
	t2 = 2 * (1 - delta) * delta * p1->y;
	t3 = pow(delta, 2) * p2->y;
	out->y = t1 + t2 + t3;
	t1 = pow(1 - delta, 2) * p0->z;
	t2 = 2 * (1 - delta) * delta * p1->z;
	t3 = pow(delta, 2) * p2->z;
	out->z = t1 + t2 + t3;
	return;
}

void Interpolate_Catmull(struct camquake_path_point *p0, struct camquake_path_point *p1, struct camquake_path_point *p2, struct camquake_path_point *p3, float time, float alpha, float tension, struct camquake_path_point *out) {
	float p0f, p1f, p2f, p3f; 
	float *outf = (float *)out;
	int i;
	float m1, m2, a, b;

	float t01 = pow(VectorDistanceQuick((vec_t *)p0, (vec_t *)p1), alpha);
	float t12 = pow(VectorDistanceQuick((vec_t *)p1, (vec_t *)p2), alpha);
	float t23 = pow(VectorDistanceQuick((vec_t *)p2, (vec_t *)p3), alpha);

	for (i=0; i<3; i++) {
		p0f = *(((float *)p0) + i);
		p1f = *(((float *)p1) + i);
		p2f = *(((float *)p2) + i);
		p3f = *(((float *)p3) + i);
		m1 = (1.0f - tension) * (p2f - p1f + t12 * ((p1f - p0f) / t01 - (p2f - p0f) / (t01 + t12)));
		m2 = (1.0f - tension) * (p2f - p1f + t12 * ((p3f - p2f) / t23 - (p3f - p1f) / (t12 + t23)));
		a = 2.0f * (p1f - p2f) + m1 + m2;
		b = -3.0f * (p1f - p2f) - m1 - m1 - m2;
		*(outf + i) = a * pow(time, 3) + b * pow(time, 2) + m1 * time + p1f;
	}
}


void CQS_Interpolate_Path(struct camquake_path_point_array *path, float t, struct camquake_path_point *out) {
	float d, da, split;
	int i;
	if (path == NULL) {
		return;
	}
	if (path->index == 1) {
		out->x = path->point[0].x;
		out->y = path->point[0].y;
		out->z = path->point[0].z;
	} else if (path->index == 2) {
		Interpolate_Linear(&path->point[0], &path->point[1], t, out);
	} else if (path->index == 3) {
		Interpolate_Bezier(&path->point[0], &path->point[1], &path->point[2], t, out);
	} else if (path->index > 3) {
		d = 0;
		i = 0;
		split = 1.0f/(path->index - 3);
		while (d <= t) {
			i++;
			d += split;
		}
		i -= 1;
		if (i < 0) { i=0; }
		if (i + 3 >= path->index) { i = path->index - 4; }
		d = i * split;
		da = (t - i * split) / split;
		Interpolate_Catmull(&path->point[i], &path->point[i+1], &path->point[i+2], &path->point[i+3], da, 0.5, 0, out);
	}
}

void CQS_Interpolate(struct camquake_setup *setup, float t, struct camquake_path_point *cam, struct camquake_path_point *view) {
	struct camquake_path_point temp;
	struct camquake_path_point_array *path;
	if (setup->camera_path.path == NULL || setup->view_path.path == NULL) {
		return;
	}
	path = setup->camera_path.path;
	CQS_Interpolate_Path(path, t, cam);
	path = setup->view_path.path;
	CQS_Interpolate_Path(path, t, view);
	temp.x = view->x - cam->x;
	temp.y = view->y - cam->y;
	temp.z = view->z - cam->z;
	Camquake_Vector_To_Angles(&temp, view);
	view->x *= -1.0f;
}

struct camquake_setup *CQS_New(struct camquake_setup **first, char *name) {
	struct camquake_setup *camquake_setup, *current;
	if (name == NULL) {
		return NULL;
	}

	if (CQS_Find(first, name) != NULL) {
		return NULL;
	}

	camquake_setup = calloc(1, sizeof(*camquake_setup));
	if (camquake_setup == NULL) {
		return NULL;
	}
	camquake_setup->name = strdup(name);
	if (camquake_setup->name == NULL) {
		free(camquake_setup);
		return NULL;
	}
	if (*first == NULL) {
		*first = camquake_setup;
	} else {
		current = *first;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = camquake_setup;
	}
	return camquake_setup;
}

struct camquake_setup *CQS_Find(struct camquake_setup **first, char *name) {
	struct camquake_setup *current;
	current = *first;
	while (current) {
		if (strcmp(current->name, name) == 0) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

