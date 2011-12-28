#ifndef type_h
#define type_h

typedef int type_id;

#define typeid(TYPE) type_find(#TYPE)
#define typename(TYPE) #TYPE

type_id type_find(const char* type);
const char* type_id_name(int id);

#endif
