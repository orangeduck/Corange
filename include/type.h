#ifndef type_h
#define type_h

typedef type_id int;

#define typeid(type) type_find(#type)

type_id type_find(const char* type);

#endif