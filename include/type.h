#ifndef type_h
#define type_h

typedef int type_id;

#define typeid(type) type_find(#type)

type_id type_find(const char* type);

#endif
