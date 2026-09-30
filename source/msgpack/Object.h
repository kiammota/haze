#ifndef OBJECT_H
#define OBJECT_H

#include "RawBuffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ObjectArray ObjectArray;
typedef struct ObjectMapEntry ObjectMap;
typedef struct ObjectMapTable ObjectMapTable;

typedef enum {
  OBJ_UND = 0,
  OBJ_NIL,
  OBJ_BOOL,
  OBJ_INT,
  OBJ_UINT,
  OBJ_FLOAT,
  OBJ_DOUBLE,
  OBJ_STR,
  OBJ_BIN,
  OBJ_ARRAY,
  OBJ_MAP,

} ObjectType;

typedef union {
  bool bool_value;
  int64_t int_value;
  uint64_t uint_value;
  float float_value;
  double double_value;
  const char *str_value;
  RawBuffer *bin_value;
  struct ObjectArray *array_value;
  ObjectMapTable *map_table_value;
  ObjectMap *map_value;
} ObjectValue;

typedef struct {
  ObjectType type;
  ObjectValue value;
  size_t size;
} Object;

// objects

Object *ObjectNew(void);
void ObjectFree(Object **ptr_to_object);
Object *ObjectCopy(const Object *v);
Object *ObjectCreateStr(const char *str);
Object *ObjectCreateBool(bool v);
Object *ObjectCreateNil(void);
Object *ObjectCreateBool(bool v);
Object *ObjectCreateInt(int64_t v);
Object *ObjectCreateUInt(uint64_t v);
Object *ObjectCreateFloat(float v);
Object *ObjectCreateDouble(double v);
Object *ObjectCreateStr(const char *str);
ObjectType ObjectGetType(const Object *t);
ObjectValue ObjectGetValue(const Object *t);
size_t ObjectGetSize(Object *t);

bool ObjectExpect(Object *t, ObjectType v);
const char *ObjectGetStr(Object *t);
bool ObjectGetBool(Object *t);
int64_t ObjectGetInt(Object *t);
uint64_t ObjectGetUInt(Object *t);
float ObjectGetFloat(Object *t);
double ObjectGetDouble(Object *t);
RawBuffer *ObjectGetBin(Object *t);
ObjectArray *ObjectGetArray(Object *t);
ObjectMap *ObjectGetMap(Object *t);
ObjectMapTable *ObjectGetMapTable(Object *t);

// TO DO: ObjectMap *ObjectGetMap(Object *t);

// arrays

#define OBJECT_ARRAY_INITIAL_CAPACITY 8

struct ObjectArray {
  Object **objects;
  size_t len;
  size_t capacity;
};

ObjectArray *ObjectArrayCreate(void);
void ObjectArrayFree(ObjectArray **array);
ObjectArray *ObjectArrayCopy(const ObjectArray *src);

bool ObjectArrayAppend(ObjectArray *array, const Object *obj);
Object *ObjectArrayGet(const ObjectArray *array, size_t index);
bool ObjectArrayRemove(ObjectArray *array, size_t index);
size_t ObjectArrayLen(const ObjectArray *array);

// maps

#define OBJECT_MAP_INITIAL_CAPACITY 8

struct ObjectMapEntry {
  Object *Key;
  Object *Value;
};

struct ObjectMapTable {
  ObjectMap **pairs;
  size_t len;
  size_t capacity;
};

ObjectMap *ObjectMapCreate(const Object *key, const Object *value);
void ObjectMapFree(ObjectMap **map);
ObjectMap *ObjectMapCopy(const ObjectMap *src);
Object* ObjectMapToObject(const ObjectMap* src);

ObjectMapTable *ObjectMapTableCreate(void);
void ObjectMapTableFree(ObjectMapTable **table);
ObjectMapTable *ObjectMapTableCopy(const ObjectMapTable *src);

bool ObjectMapTableSet(ObjectMapTable *table, const Object *key,
                       const Object *value);
Object *ObjectMapTableGet(const ObjectMapTable *table, const Object *key);
bool ObjectMapTableRemove(ObjectMapTable *table, const Object *key);
size_t ObjectMapTableLen(const ObjectMapTable *table);
Object *ObjectMapTableToObject(const ObjectMapTable *table);

#endif
