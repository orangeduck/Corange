#ifndef obj_loader_h
#define obj_loader_h

#include "geometry.h"

/* Loads an obj file into a model */
render_model* obj_load_file(char* filename);

/* Loads an obj object from a working string */
model* obj_load_object(char* contents);

/* Parses a triangle entry in an obj file */
void obj_parse_triangle(char* line, vector3* positions, vector3* normals, vector2* uvs, vertex* v1, vertex* v2, vertex* v3);

#endif