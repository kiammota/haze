#include "Object.h"
#include "HazeMacros.h"
#include "RawBuffer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Declare o protótipo antes de usar no ObjectCopy
ObjectArray *ObjectArrayCopy(const ObjectArray *src);

Object *ObjectNew(void) {
  Object *obj = malloc(sizeof(Object));
  if (!obj)
    return NULL;

  obj->size = 0;
  obj->type = OBJ_UND;
  memset(&obj->value, 0, sizeof(ObjectValue));
  return obj;
}

void ObjectFree(Object **ptr_to_object) {
  PTR_FREE_ASSERT(ptr_to_object);

  Object *obj = *ptr_to_object;

  switch (obj->type) {
  case OBJ_STR:
    if (obj->value.str_value) {
      free((void *)obj->value.str_value);
    }
    break;

  case OBJ_BIN:
    if (obj->value.bin_value) {
      free(obj->value.bin_value);
    }
    break;

  case OBJ_ARRAY:
    if (obj->value.array_value) {
      // CORREÇÃO: Libera a memória interna do array
      ObjectArrayFree(&obj->value.array_value);
    }
    break;

  case OBJ_MAP:
    break;

  default:
    break;
  }

  free(obj);
  *ptr_to_object = NULL;
}

Object *ObjectCopy(const Object *v) {
  if (!v)
    return NULL;

  Object *copy = ObjectNew();
  if (!copy)
    return NULL;

  copy->size = v->size;
  copy->type = v->type;

  switch (v->type) {
  case OBJ_UND:
  case OBJ_NIL:
    break;

  case OBJ_BOOL:
    copy->value.bool_value = v->value.bool_value;
    break;

  case OBJ_INT:
    copy->value.int_value = v->value.int_value;
    break;

  case OBJ_UINT:
    copy->value.uint_value = v->value.uint_value;
    break;

  case OBJ_FLOAT:
    copy->value.float_value = v->value.float_value;
    break;

  case OBJ_DOUBLE:
    copy->value.double_value = v->value.double_value;
    break;

  case OBJ_STR:
    if (v->value.str_value) {
      copy->value.str_value = strdup(v->value.str_value);
      if (!copy->value.str_value) {
        free(copy);
        return NULL;
      }
    } else {
      copy->value.str_value = NULL;
    }
    break;

  case OBJ_BIN:
    if (v->value.bin_value) {
      copy->value.bin_value = RawBufferDup(v->value.bin_value);

      if (!copy->value.bin_value) {
        free(copy);
        return NULL;
      }
    } else {
      copy->value.bin_value = NULL;
    }
    break;

  case OBJ_ARRAY:
    if (v->value.array_value) {
      copy->value.array_value = ObjectArrayCopy(v->value.array_value);
      if (!copy->value.array_value) {
        free(copy);
        return NULL;
      }
    } else {
      copy->value.array_value = NULL;
    }
    break;

  case OBJ_MAP:
    break;

  default:
    break;
  }

  return copy;
}

Object *ObjectCreateStr(const char *str) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;

  // CORREÇÃO: Trata ponteiro NULL antes de chamar strdup
  char *strCopy = strdup(str ? str : "NULL");
  if (!strCopy) {
    free(myObj);
    return NULL;
  }

  myObj->type = OBJ_STR;
  myObj->value.str_value = strCopy;
  myObj->size = strlen(strCopy);
  return myObj;
}

Object *ObjectCreateBool(bool v) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_BOOL;
  myObj->size = sizeof(bool);
  myObj->value.bool_value = v;
  return myObj;
}

Object *ObjectCreateNil(void) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_NIL;
  myObj->size = 0;
  return myObj;
}

Object *ObjectCreateInt(int64_t v) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_INT;
  myObj->size = sizeof(int64_t);
  myObj->value.int_value = v;
  return myObj;
}

Object *ObjectCreateUInt(uint64_t v) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_UINT;
  myObj->size = sizeof(uint64_t);
  myObj->value.uint_value = v;
  return myObj;
}

Object *ObjectCreateFloat(float v) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_FLOAT;
  myObj->size = sizeof(float);
  myObj->value.float_value = v;
  return myObj;
}

Object *ObjectCreateDouble(double v) {
  Object *myObj = ObjectNew();
  if (!myObj)
    return NULL;
  myObj->type = OBJ_DOUBLE;
  myObj->size = sizeof(double);
  myObj->value.double_value = v;
  return myObj;
}

ObjectType ObjectGetType(const Object *t) {
  if (!t)
    return OBJ_UND;
  return t->type;
}

ObjectValue ObjectGetValue(const Object *t) {
  ObjectValue value = {0};
  if (!t)
    return value;
  return t->value;
}

size_t ObjectGetSize(Object *t) {
  if (!t)
    return 0;
  return t->size;
}

bool ObjectExpect(Object *t, ObjectType type) {
  if (!t)
    return false;
  return t->type == type;
}

const char *ObjectGetStr(Object *t) { return t->value.str_value; }

bool ObjectGetBool(Object *t) { return t->value.bool_value; }

int64_t ObjectGetInt(Object *t) { return t->value.int_value; }

uint64_t ObjectGetUInt(Object *t) { return t->value.uint_value; }

float ObjectGetFloat(Object *t) { return t->value.float_value; }

double ObjectGetDouble(Object *t) { return t->value.double_value; }

RawBuffer *ObjectGetBin(Object *t) { return t->value.bin_value; }

ObjectArray *ObjectGetArray(Object *t) { return t->value.array_value; }

// --- Operações de Array ---

ObjectArray *ObjectArrayCreate(void) {
  ObjectArray *array = (ObjectArray *)malloc(sizeof(ObjectArray));
  if (!array)
    return NULL;

  array->capacity = OBJECT_ARRAY_INITIAL_CAPACITY;
  array->len = 0;
  array->objects = (Object **)malloc(sizeof(Object *) * array->capacity);

  if (!array->objects) {
    free(array);
    return NULL;
  }

  return array;
}

void ObjectArrayFree(ObjectArray **array) {
  if (!array || !*array)
    return;

  ObjectArray *arr = *array;
  for (size_t i = 0; i < arr->len; i++) {
    if (arr->objects[i]) {
      ObjectFree(&arr->objects[i]);
    }
  }

  free(arr->objects);
  free(arr);
  *array = NULL;
}

ObjectArray *ObjectArrayCopy(const ObjectArray *src) {
  if (!src)
    return NULL;

  ObjectArray *dst = ObjectArrayCreate();
  if (!dst)
    return NULL;

  for (size_t i = 0; i < src->len; i++) {
    if (!ObjectArrayAppend(dst, src->objects[i])) {
      ObjectArrayFree(&dst);
      return NULL;
    }
  }

  return dst;
}

bool ObjectArrayAppend(ObjectArray *array, const Object *obj) {
  if (!array || !obj)
    return false;

  if (array->len >= array->capacity) {
    size_t new_capacity = array->capacity * 2;
    Object **new_objects =
        (Object **)realloc(array->objects, sizeof(Object *) * new_capacity);
    if (!new_objects)
      return false;

    array->objects = new_objects;
    array->capacity = new_capacity;
  }

  Object *copy = ObjectCopy(obj);
  if (!copy)
    return false;

  array->objects[array->len] = copy;
  array->len++;

  return true;
}

Object *ObjectArrayGet(const ObjectArray *array, size_t index) {
  if (!array || index >= array->len)
    return NULL;
  return array->objects[index];
}

bool ObjectArrayRemove(ObjectArray *array, size_t index) {
  if (!array || index >= array->len)
    return false;

  ObjectFree(&array->objects[index]);

  for (size_t i = index; i < array->len - 1; i++) {
    array->objects[i] = array->objects[i + 1];
  }

  array->len--;
  array->objects[array->len] = NULL;

  return true;
}

size_t ObjectArrayLen(const ObjectArray *array) {
  if (!array)
    return 0;
  return array->len;
}

// --- Operações de Map ---

ObjectMap *ObjectMapCreate(const Object *key, const Object *value) {
  if (!key || !value)
    return NULL;

  ObjectMap *map = (ObjectMap *)malloc(sizeof(ObjectMap));
  if (!map)
    return NULL;

  map->Key = ObjectCopy(key);
  map->Value = ObjectCopy(value);

  if (!map->Key || !map->Value) {
    ObjectFree(&map->Key);
    ObjectFree(&map->Value);
    free(map);
    return NULL;
  }

  return map;
}

void ObjectMapFree(ObjectMap **map) {
  if (!map || !*map)
    return;

  ObjectMap *m = *map;
  ObjectFree(&m->Key);
  ObjectFree(&m->Value);
  free(m);
  *map = NULL;
}

ObjectMap *ObjectMapCopy(const ObjectMap *src) {
  if (!src)
    return NULL;
  return ObjectMapCreate(src->Key, src->Value);
}

// --- Comparação Auxiliar de Chaves ---

static bool ObjectEquals(const Object *a, const Object *b) {
  if (!a || !b)
    return false;
  if (a->type != b->type)
    return false;

  switch (a->type) {
  case OBJ_BOOL:
    return a->value.bool_value == b->value.bool_value;
  case OBJ_INT:
    return a->value.int_value == b->value.int_value;
  case OBJ_UINT:
    return a->value.uint_value == b->value.uint_value;
  case OBJ_FLOAT:
    return a->value.float_value == b->value.float_value;
  case OBJ_DOUBLE:
    return a->value.double_value == b->value.double_value;
  case OBJ_STR:
    if (!a->value.str_value || !b->value.str_value)
      return a->value.str_value == b->value.str_value;
    return strcmp(a->value.str_value, b->value.str_value) == 0;
  default:
    return false;
  }
}

// --- Funções da Tabela de Mapa (ObjectMapTable) ---

ObjectMapTable *ObjectMapTableCreate(void) {
  ObjectMapTable *table = (ObjectMapTable *)malloc(sizeof(ObjectMapTable));
  if (!table)
    return NULL;

  table->capacity = OBJECT_MAP_INITIAL_CAPACITY;
  table->len = 0;
  table->pairs = (ObjectMap **)malloc(sizeof(ObjectMap *) * table->capacity);

  if (!table->pairs) {
    free(table);
    return NULL;
  }

  return table;
}

void ObjectMapTableFree(ObjectMapTable **table) {
  if (!table || !*table)
    return;

  ObjectMapTable *tbl = *table;
  for (size_t i = 0; i < tbl->len; i++) {
    if (tbl->pairs[i]) {
      ObjectMapFree(&tbl->pairs[i]);
    }
  }

  free(tbl->pairs);
  free(tbl);
  *table = NULL;
}

ObjectMapTable *ObjectMapTableCopy(const ObjectMapTable *src) {
  if (!src)
    return NULL;

  ObjectMapTable *dst = ObjectMapTableCreate();
  if (!dst)
    return NULL;

  for (size_t i = 0; i < src->len; i++) {
    if (!ObjectMapTableSet(dst, src->pairs[i]->Key, src->pairs[i]->Value)) {
      ObjectMapTableFree(&dst);
      return NULL;
    }
  }

  return dst;
}

bool ObjectMapTableSet(ObjectMapTable *table, const Object *key,
                       const Object *value) {
  if (!table || !key || !value)
    return false;

  // Se a chave já existir, atualiza o valor correspondente
  for (size_t i = 0; i < table->len; i++) {
    if (ObjectEquals(table->pairs[i]->Key, key)) {
      Object *new_value = ObjectCopy(value);
      if (!new_value)
        return false;

      ObjectFree(&table->pairs[i]->Value);
      table->pairs[i]->Value = new_value;
      return true;
    }
  }

  // Expandir capacidade se estiver cheio
  if (table->len >= table->capacity) {
    size_t new_capacity = table->capacity * 2;
    ObjectMap **new_pairs =
        (ObjectMap **)realloc(table->pairs, sizeof(ObjectMap *) * new_capacity);
    if (!new_pairs)
      return false;

    table->pairs = new_pairs;
    table->capacity = new_capacity;
  }

  // Inserir novo par
  ObjectMap *pair = ObjectMapCreate(key, value);
  if (!pair)
    return false;

  table->pairs[table->len] = pair;
  table->len++;

  return true;
}

Object *ObjectMapTableGet(const ObjectMapTable *table, const Object *key) {
  if (!table || !key)
    return NULL;

  for (size_t i = 0; i < table->len; i++) {
    if (ObjectEquals(table->pairs[i]->Key, key)) {
      return table->pairs[i]->Value;
    }
  }

  return NULL;
}

bool ObjectMapTableRemove(ObjectMapTable *table, const Object *key) {
  if (!table || !key)
    return false;

  for (size_t i = 0; i < table->len; i++) {
    if (ObjectEquals(table->pairs[i]->Key, key)) {
      ObjectMapFree(&table->pairs[i]);

      for (size_t j = i; j < table->len - 1; j++) {
        table->pairs[j] = table->pairs[j + 1];
      }

      table->len--;
      table->pairs[table->len] = NULL;
      return true;
    }
  }

  return false;
}

size_t ObjectMapTableLen(const ObjectMapTable *table) {
  if (!table)
    return 0;
  return table->len;
}
