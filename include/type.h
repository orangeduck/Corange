#ifndef type_h
#define type_h

#define typeid(type) type_find(#type)

int type_find(const char* type);

#endif