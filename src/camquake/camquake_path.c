#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "camquake_internal.h"
#include "../common.h"

struct camquake_path_point *CQP_Point_New(struct camquake_path_point_array **path_point_array)
{
	unsigned int size = 32;
	if (*path_point_array == NULL) {
		*path_point_array = calloc(1, sizeof(struct camquake_path_point_array) + sizeof(struct camquake_path_point) * (size - 1 ));
		(*path_point_array)->size = size;
		(*path_point_array)->index = 0;
	}
	if ((*path_point_array)->index == (*path_point_array)->size) {
		*path_point_array = realloc(*path_point_array, sizeof(struct camquake_path_point_array) + sizeof(struct camquake_path_point) * ((*path_point_array)->size  + size - 1));
		(*path_point_array)->size += size;
	}
	return &((*path_point_array)->point[(*path_point_array)->index++]);
}


struct camquake_path_point *CQP_Point_New_Index(struct camquake_path_point_array **path_point_array, unsigned int index)
{
	int i;
	if ((*path_point_array) == NULL) {
		return NULL;
	}
	if ((*path_point_array)->index < index) {
		return CQP_Point_New(path_point_array);
	}

	CQP_Point_New(path_point_array);
	for (i=(*path_point_array)->index - 2; i>= index && i >= 0; i--) {
		memcpy(&(*path_point_array)->point[i+1],
		&(*path_point_array)->point[i],
		sizeof(*(*path_point_array)->point));
	}
	memset(&(*path_point_array)->point[index], 0, sizeof(*(*path_point_array)->point));
	return (&(*path_point_array)->point[index]);
}


int CQP_Point_Remove_Index(struct camquake_path_point_array **path_point_array, unsigned int position)
{
	int i;
	if ((*path_point_array) == NULL) {
		return 1;
	}
	if ((*path_point_array)->index <= position) {
		return 1;
	}

	for (i=position + 1; i<(*path_point_array)->index; i++) {
		memcpy(&(*path_point_array)->point[i-1],
		&(*path_point_array)->point[i],
		sizeof(*(*path_point_array)->point));
	}
	(*path_point_array)->index--;
	if ((*path_point_array)->index < 0) {
		(*path_point_array)->index = 0;
	}

	return 0;
}

void CQP_Free(struct camquake_path *path) {
	free(path->path);
	free(path->interpolated_array);
	free(path->projected_points);
}

void CQP_Print(struct camquake_path_point_array *path_point_array)
{
	int i;
	struct camquake_path_point *point;
	if (path_point_array == NULL) {
		return;
	}
	Com_Printf("Size: %d\n", path_point_array->size);
	Com_Printf("Length: %d\n", path_point_array->index);
	for (i=0; i<path_point_array->index; i++) {
		point = &path_point_array->point[i];
		Com_Printf("point %d: %f %f %f\n", i, point->x, point->y, point->z);
	}
}


void Camquake_Vector_To_Angles(struct camquake_path_point *vec, struct camquake_path_point *ang) 
{
	float forward, yaw, pitch;
	yaw = (atan2(vec->y, vec->x) * 180 / M_PI);
	if (yaw < 0)
		yaw += 360;

	forward = sqrt (vec->x*vec->x + vec->y*vec->y);
	pitch =(atan2(vec->z, forward) * 180 / M_PI);

	ang->x = pitch;
	ang->y = yaw;
	ang->z = 0;
}
