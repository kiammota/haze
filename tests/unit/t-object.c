#include "msgpack/Object.h"
#include "macros.h"

#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int TestObjectScalars(void) {
    TEST_LOG("Object scalar types");

    Object *nil = ObjectCreateNil();
    TEST_ASSERT(nil != NULL);
    TEST_ASSERT(ObjectGetType(nil) == OBJ_NIL);
    TEST_ASSERT(ObjectExpect(nil, OBJ_NIL));
    TEST_ASSERT(!ObjectExpect(nil, OBJ_BOOL));
    ObjectFree(&nil);
    TEST_ASSERT(nil == NULL);

    Object *boolean = ObjectCreateBool(true);
    TEST_ASSERT(boolean != NULL);
    TEST_ASSERT(ObjectGetType(boolean) == OBJ_BOOL);
    TEST_ASSERT(ObjectGetBool(boolean) == true);
    TEST_ASSERT(ObjectExpect(boolean, OBJ_BOOL));
    TEST_ASSERT(!ObjectExpect(boolean, OBJ_INT));
    ObjectFree(&boolean);

    Object *integer = ObjectCreateInt(INT64_MIN);
    TEST_ASSERT(integer != NULL);
    TEST_ASSERT(ObjectGetType(integer) == OBJ_INT);
    TEST_ASSERT(ObjectGetInt(integer) == INT64_MIN);
    TEST_ASSERT(ObjectGetValue(integer).int_value == INT64_MIN);
    ObjectFree(&integer);

    integer = ObjectCreateInt(INT64_MAX);
    TEST_ASSERT(integer != NULL);
    TEST_ASSERT(ObjectGetInt(integer) == INT64_MAX);
    ObjectFree(&integer);

    Object *uinteger = ObjectCreateUInt(UINT64_MAX);
    TEST_ASSERT(uinteger != NULL);
    TEST_ASSERT(ObjectGetType(uinteger) == OBJ_UINT);
    TEST_ASSERT(ObjectGetUInt(uinteger) == UINT64_MAX);
    TEST_ASSERT(ObjectGetValue(uinteger).uint_value == UINT64_MAX);
    ObjectFree(&uinteger);

    Object *floating = ObjectCreateFloat(FLT_MAX);
    TEST_ASSERT(floating != NULL);
    TEST_ASSERT(ObjectGetType(floating) == OBJ_FLOAT);
    TEST_ASSERT(ObjectGetFloat(floating) == FLT_MAX);
    TEST_ASSERT(ObjectGetValue(floating).float_value == FLT_MAX);
    ObjectFree(&floating);

    Object *doubling = ObjectCreateDouble(DBL_MAX);
    TEST_ASSERT(doubling != NULL);
    TEST_ASSERT(ObjectGetType(doubling) == OBJ_DOUBLE);
    TEST_ASSERT(ObjectGetDouble(doubling) == DBL_MAX);
    TEST_ASSERT(ObjectGetValue(doubling).double_value == DBL_MAX);
    ObjectFree(&doubling);

    TEST_PASS();
}

static int TestObjectStrings(void) {
    TEST_LOG("Object strings");

    const char *text = "Haze Object stress test";

    Object *object = ObjectCreateStr(text);
    TEST_ASSERT(object != NULL);
    TEST_ASSERT(ObjectGetType(object) == OBJ_STR);
    TEST_ASSERT(ObjectExpect(object, OBJ_STR));
    TEST_ASSERT(!ObjectExpect(object, OBJ_BIN));

    const char *result = ObjectGetStr(object);

    TEST_ASSERT(result != NULL);
    TEST_ASSERT(strcmp(result, text) == 0);
    TEST_ASSERT(ObjectGetValue(object).str_value == result);
    TEST_ASSERT(ObjectGetSize(object) == strlen(text));

    Object *empty = ObjectCreateStr("");
    TEST_ASSERT(empty != NULL);
    TEST_ASSERT(ObjectGetStr(empty) != NULL);
    TEST_ASSERT(strcmp(ObjectGetStr(empty), "") == 0);
    TEST_ASSERT(ObjectGetSize(empty) == 0);

    ObjectFree(&object);
    ObjectFree(&empty);

    TEST_ASSERT(object == NULL);
    TEST_ASSERT(empty == NULL);

    TEST_PASS();
}

static int TestObjectCopy(void) {
    TEST_LOG("ObjectCopy");

    Object *original = ObjectCreateStr("copy me");
    TEST_ASSERT(original != NULL);

    Object *copy = ObjectCopy(original);
    TEST_ASSERT(copy != NULL);

    TEST_ASSERT(copy != original);
    TEST_ASSERT(ObjectGetType(copy) == ObjectGetType(original));
    TEST_ASSERT(ObjectGetSize(copy) == ObjectGetSize(original));
    TEST_ASSERT(strcmp(ObjectGetStr(copy), ObjectGetStr(original)) == 0);

    ObjectFree(&original);

    TEST_ASSERT(original == NULL);
    TEST_ASSERT(strcmp(ObjectGetStr(copy), "copy me") == 0);

    ObjectFree(&copy);

    for (int i = 0; i < 10000; i++) {
        Object *src = ObjectCreateInt(i);
        TEST_ASSERT(src != NULL);

        Object *dst = ObjectCopy(src);
        TEST_ASSERT(dst != NULL);

        TEST_ASSERT(dst != src);
        TEST_ASSERT(ObjectGetType(dst) == OBJ_INT);
        TEST_ASSERT(ObjectGetInt(dst) == i);

        ObjectFree(&src);
        TEST_ASSERT(src == NULL);

        TEST_ASSERT(ObjectGetInt(dst) == i);

        ObjectFree(&dst);
        TEST_ASSERT(dst == NULL);
    }

    TEST_PASS();
}

static int TestObjectArrayBasic(void) {
    TEST_LOG("ObjectArray basic operations");

    ObjectArray *array = ObjectArrayCreate();
    TEST_ASSERT(array != NULL);

    TEST_ASSERT(ObjectArrayLen(array) == 0);

    Object *a = ObjectCreateInt(10);
    Object *b = ObjectCreateStr("hello");
    Object *c = ObjectCreateBool(true);

    TEST_ASSERT(a != NULL);
    TEST_ASSERT(b != NULL);
    TEST_ASSERT(c != NULL);

    TEST_ASSERT(ObjectArrayAppend(array, a));
    TEST_ASSERT(ObjectArrayAppend(array, b));
    TEST_ASSERT(ObjectArrayAppend(array, c));

    TEST_ASSERT(ObjectArrayLen(array) == 3);

    TEST_ASSERT(ObjectArrayGet(array, 0) != NULL);
    TEST_ASSERT(ObjectArrayGet(array, 1) != NULL);
    TEST_ASSERT(ObjectArrayGet(array, 2) != NULL);

    TEST_ASSERT(ObjectGetInt(ObjectArrayGet(array, 0)) == 10);
    TEST_ASSERT(strcmp(ObjectGetStr(ObjectArrayGet(array, 1)), "hello") == 0);
    TEST_ASSERT(ObjectGetBool(ObjectArrayGet(array, 2)) == true);

    TEST_ASSERT(ObjectArrayGet(array, 3) == NULL);
    TEST_ASSERT(ObjectArrayGet(array, SIZE_MAX) == NULL);

    /*
     * A good container implementation should own its inserted objects.
     * Therefore destroying the originals must not invalidate the array.
     */
    ObjectFree(&a);
    ObjectFree(&b);
    ObjectFree(&c);

    TEST_ASSERT(ObjectGetInt(ObjectArrayGet(array, 0)) == 10);
    TEST_ASSERT(strcmp(ObjectGetStr(ObjectArrayGet(array, 1)), "hello") == 0);
    TEST_ASSERT(ObjectGetBool(ObjectArrayGet(array, 2)) == true);

    TEST_ASSERT(ObjectArrayRemove(array, 1));
    TEST_ASSERT(ObjectArrayLen(array) == 2);

    TEST_ASSERT(ObjectGetInt(ObjectArrayGet(array, 0)) == 10);
    TEST_ASSERT(ObjectGetBool(ObjectArrayGet(array, 1)) == true);

    TEST_ASSERT(!ObjectArrayRemove(array, 99));
    TEST_ASSERT(!ObjectArrayRemove(array, SIZE_MAX));

    ObjectArrayFree(&array);
    TEST_ASSERT(array == NULL);

    TEST_PASS();
}

static int TestObjectArrayGrowth(void) {
    TEST_LOG("ObjectArray growth");

    ObjectArray *array = ObjectArrayCreate();
    TEST_ASSERT(array != NULL);

    const size_t count = 100000;

    for (size_t i = 0; i < count; i++) {
        Object *object = ObjectCreateUInt(i);
        TEST_ASSERT(object != NULL);

        TEST_ASSERT(ObjectArrayAppend(array, object));

        ObjectFree(&object);
        TEST_ASSERT(object == NULL);
    }

    TEST_ASSERT(ObjectArrayLen(array) == count);

    for (size_t i = 0; i < count; i++) {
        Object *object = ObjectArrayGet(array, i);

        TEST_ASSERT(object != NULL);
        TEST_ASSERT(ObjectGetType(object) == OBJ_UINT);
        TEST_ASSERT(ObjectGetUInt(object) == i);
    }

    ObjectArrayFree(&array);

    TEST_ASSERT(array == NULL);

    TEST_PASS();
}

static int TestObjectArrayCopy(void) {
    TEST_LOG("ObjectArrayCopy");

    ObjectArray *original = ObjectArrayCreate();
    TEST_ASSERT(original != NULL);

    for (int i = 0; i < 1000; i++) {
        Object *object = ObjectCreateInt(i);
        TEST_ASSERT(object != NULL);

        TEST_ASSERT(ObjectArrayAppend(original, object));

        ObjectFree(&object);
    }

    ObjectArray *copy = ObjectArrayCopy(original);
    TEST_ASSERT(copy != NULL);

    TEST_ASSERT(copy != original);
    TEST_ASSERT(ObjectArrayLen(copy) == ObjectArrayLen(original));

    for (size_t i = 0; i < ObjectArrayLen(original); i++) {
        Object *a = ObjectArrayGet(original, i);
        Object *b = ObjectArrayGet(copy, i);

        TEST_ASSERT(a != NULL);
        TEST_ASSERT(b != NULL);
        TEST_ASSERT(a != b);
        TEST_ASSERT(ObjectGetInt(a) == ObjectGetInt(b));
    }

    Object *extra = ObjectCreateInt(999999);
    TEST_ASSERT(extra != NULL);

    TEST_ASSERT(ObjectArrayAppend(original, extra));
    ObjectFree(&extra);

    TEST_ASSERT(ObjectArrayLen(original) == 1001);
    TEST_ASSERT(ObjectArrayLen(copy) == 1000);

    TEST_ASSERT(ObjectArrayRemove(original, 0));
    TEST_ASSERT(ObjectArrayLen(original) == 1000);
    TEST_ASSERT(ObjectArrayLen(copy) == 1000);

    ObjectArrayFree(&original);
    TEST_ASSERT(original == NULL);

    TEST_ASSERT(ObjectArrayLen(copy) == 1000);
    TEST_ASSERT(ObjectGetInt(ObjectArrayGet(copy, 0)) == 0);

    ObjectArrayFree(&copy);
    TEST_ASSERT(copy == NULL);

    TEST_PASS();
}

static int TestObjectMapBasic(void) {
    TEST_LOG("ObjectMap basic operations");

    Object *key = ObjectCreateStr("name");
    Object *value = ObjectCreateStr("Haze");

    TEST_ASSERT(key != NULL);
    TEST_ASSERT(value != NULL);

    ObjectMap *pair = ObjectMapCreate(key, value);
    TEST_ASSERT(pair != NULL);
    TEST_ASSERT(pair->Key != NULL);
    TEST_ASSERT(pair->Value != NULL);

    TEST_ASSERT(strcmp(ObjectGetStr(pair->Key), "name") == 0);
    TEST_ASSERT(strcmp(ObjectGetStr(pair->Value), "Haze") == 0);

    ObjectFree(&key);
    ObjectFree(&value);

    TEST_ASSERT(strcmp(ObjectGetStr(pair->Key), "name") == 0);
    TEST_ASSERT(strcmp(ObjectGetStr(pair->Value), "Haze") == 0);

    ObjectMap *copy = ObjectMapCopy(pair);
    TEST_ASSERT(copy != NULL);
    TEST_ASSERT(copy != pair);

    TEST_ASSERT(copy->Key != pair->Key);
    TEST_ASSERT(copy->Value != pair->Value);

    TEST_ASSERT(strcmp(ObjectGetStr(copy->Key), "name") == 0);
    TEST_ASSERT(strcmp(ObjectGetStr(copy->Value), "Haze") == 0);

    ObjectMapFree(&pair);
    ObjectMapFree(&copy);

    TEST_ASSERT(pair == NULL);
    TEST_ASSERT(copy == NULL);

    TEST_PASS();
}

static int TestObjectMapTable(void) {
    TEST_LOG("ObjectMapTable");

    ObjectMapTable *table = ObjectMapTableCreate();
    TEST_ASSERT(table != NULL);

    TEST_ASSERT(ObjectMapTableLen(table) == 0);

    for (int i = 0; i < 10000; i++) {
        Object *key = ObjectCreateInt(i);
        Object *value = ObjectCreateUInt((uint64_t)i * 10);

        TEST_ASSERT(key != NULL);
        TEST_ASSERT(value != NULL);

        TEST_ASSERT(ObjectMapTableSet(table, key, value));

        ObjectFree(&key);
        ObjectFree(&value);
    }

    TEST_ASSERT(ObjectMapTableLen(table) == 10000);

    for (int i = 0; i < 10000; i++) {
        Object *key = ObjectCreateInt(i);
        TEST_ASSERT(key != NULL);

        Object *value = ObjectMapTableGet(table, key);

        TEST_ASSERT(value != NULL);
        TEST_ASSERT(ObjectGetType(value) == OBJ_UINT);
        TEST_ASSERT(ObjectGetUInt(value) == (uint64_t)i * 10);

        ObjectFree(&key);
    }

    Object *missing = ObjectCreateInt(999999999);
    TEST_ASSERT(missing != NULL);

    TEST_ASSERT(ObjectMapTableGet(table, missing) == NULL);

    ObjectFree(&missing);

    /*
     * Replace an existing key.
     */
    Object *key = ObjectCreateInt(5000);
    Object *value = ObjectCreateUInt(123456789);

    TEST_ASSERT(key != NULL);
    TEST_ASSERT(value != NULL);

    TEST_ASSERT(ObjectMapTableSet(table, key, value));

    ObjectFree(&key);
    ObjectFree(&value);

    key = ObjectCreateInt(5000);
    TEST_ASSERT(key != NULL);

    value = ObjectMapTableGet(table, key);
    TEST_ASSERT(value != NULL);
    TEST_ASSERT(ObjectGetUInt(value) == 123456789);

    ObjectFree(&key);

    /*
     * Remove half of the table.
     */
    for (int i = 0; i < 10000; i += 2) {
        key = ObjectCreateInt(i);
        TEST_ASSERT(key != NULL);

        TEST_ASSERT(ObjectMapTableRemove(table, key));

        ObjectFree(&key);
    }

    TEST_ASSERT(ObjectMapTableLen(table) == 5000);

    for (int i = 0; i < 10000; i++) {
        key = ObjectCreateInt(i);
        TEST_ASSERT(key != NULL);

        value = ObjectMapTableGet(table, key);

        if (i % 2 == 0) {
            TEST_ASSERT(value == NULL);
        } else {
            TEST_ASSERT(value != NULL);
            TEST_ASSERT(ObjectGetUInt(value) == (uint64_t)i * 10);
        }

        ObjectFree(&key);
    }

    /*
     * Removing something that no longer exists must fail cleanly.
     */
    key = ObjectCreateInt(0);
    TEST_ASSERT(key != NULL);
    TEST_ASSERT(!ObjectMapTableRemove(table, key));
    ObjectFree(&key);

    ObjectMapTableFree(&table);
    TEST_ASSERT(table == NULL);

    TEST_PASS();
}

static int TestObjectMapCopy(void) {
    TEST_LOG("ObjectMapTableCopy");

    ObjectMapTable *original = ObjectMapTableCreate();
    TEST_ASSERT(original != NULL);

    for (int i = 0; i < 5000; i++) {
        Object *key = ObjectCreateInt(i);
        Object *value = ObjectCreateStr("value");

        TEST_ASSERT(key != NULL);
        TEST_ASSERT(value != NULL);

        TEST_ASSERT(ObjectMapTableSet(original, key, value));

        ObjectFree(&key);
        ObjectFree(&value);
    }

    ObjectMapTable *copy = ObjectMapTableCopy(original);

    TEST_ASSERT(copy != NULL);
    TEST_ASSERT(copy != original);
    TEST_ASSERT(ObjectMapTableLen(copy) == ObjectMapTableLen(original));

    for (int i = 0; i < 5000; i++) {
        Object *key = ObjectCreateInt(i);

        TEST_ASSERT(key != NULL);

        Object *a = ObjectMapTableGet(original, key);
        Object *b = ObjectMapTableGet(copy, key);

        TEST_ASSERT(a != NULL);
        TEST_ASSERT(b != NULL);
        TEST_ASSERT(a != b);
        TEST_ASSERT(strcmp(ObjectGetStr(a), ObjectGetStr(b)) == 0);

        ObjectFree(&key);
    }

    Object *newKey = ObjectCreateInt(999999);
    Object *newValue = ObjectCreateStr("new");

    TEST_ASSERT(newKey != NULL);
    TEST_ASSERT(newValue != NULL);

    TEST_ASSERT(ObjectMapTableSet(original, newKey, newValue));

    ObjectFree(&newKey);
    ObjectFree(&newValue);

    TEST_ASSERT(ObjectMapTableLen(original) == 5001);
    TEST_ASSERT(ObjectMapTableLen(copy) == 5000);

    ObjectMapTableFree(&original);

    TEST_ASSERT(original == NULL);
    TEST_ASSERT(ObjectMapTableLen(copy) == 5000);

    ObjectMapTableFree(&copy);

    TEST_ASSERT(copy == NULL);

    TEST_PASS();
}

static int TestObjectStressMixed(void) {
    TEST_LOG("Mixed stress");

    ObjectArray *array = ObjectArrayCreate();
    ObjectMapTable *table = ObjectMapTableCreate();

    TEST_ASSERT(array != NULL);
    TEST_ASSERT(table != NULL);

    for (int i = 0; i < 20000; i++) {
        Object *object = NULL;

        switch (i % 7) {
        case 0:
            object = ObjectCreateNil();
            break;

        case 1:
            object = ObjectCreateBool(i % 2 == 0);
            break;

        case 2:
            object = ObjectCreateInt(-(int64_t)i);
            break;

        case 3:
            object = ObjectCreateUInt((uint64_t)i * 100);
            break;

        case 4:
            object = ObjectCreateFloat((float)i);
            break;

        case 5:
            object = ObjectCreateDouble((double)i);
            break;

        case 6:
            object = ObjectCreateStr("stress");
            break;
        }

        TEST_ASSERT(object != NULL);
        TEST_ASSERT(ObjectArrayAppend(array, object));

        ObjectFree(&object);
    }

    TEST_ASSERT(ObjectArrayLen(array) == 20000);

    for (int i = 0; i < 20000; i++) {
        Object *object = ObjectArrayGet(array, (size_t)i);
        TEST_ASSERT(object != NULL);

        switch (i % 7) {
        case 0:
            TEST_ASSERT(ObjectGetType(object) == OBJ_NIL);
            break;

        case 1:
            TEST_ASSERT(ObjectGetType(object) == OBJ_BOOL);
            TEST_ASSERT(ObjectGetBool(object) == (i % 2 == 0));
            break;

        case 2:
            TEST_ASSERT(ObjectGetType(object) == OBJ_INT);
            TEST_ASSERT(ObjectGetInt(object) == -(int64_t)i);
            break;

        case 3:
            TEST_ASSERT(ObjectGetType(object) == OBJ_UINT);
            TEST_ASSERT(ObjectGetUInt(object) == (uint64_t)i * 100);
            break;

        case 4:
            TEST_ASSERT(ObjectGetType(object) == OBJ_FLOAT);
            TEST_ASSERT(ObjectGetFloat(object) == (float)i);
            break;

        case 5:
            TEST_ASSERT(ObjectGetType(object) == OBJ_DOUBLE);
            TEST_ASSERT(ObjectGetDouble(object) == (double)i);
            break;

        case 6:
            TEST_ASSERT(ObjectGetType(object) == OBJ_STR);
            TEST_ASSERT(strcmp(ObjectGetStr(object), "stress") == 0);
            break;
        }
    }

    for (int i = 0; i < 10000; i++) {
        Object *key = ObjectCreateUInt(i);
        Object *value = ObjectCreateInt(-i);

        TEST_ASSERT(key != NULL);
        TEST_ASSERT(value != NULL);

        TEST_ASSERT(ObjectMapTableSet(table, key, value));

        ObjectFree(&key);
        ObjectFree(&value);
    }

    TEST_ASSERT(ObjectMapTableLen(table) == 10000);

    for (int i = 0; i < 5000; i++) {
        Object *key = ObjectCreateUInt(i);
        TEST_ASSERT(key != NULL);

        TEST_ASSERT(ObjectMapTableRemove(table, key));

        ObjectFree(&key);
    }

    TEST_ASSERT(ObjectMapTableLen(table) == 5000);

    ObjectArrayFree(&array);
    ObjectMapTableFree(&table);

    TEST_ASSERT(array == NULL);
    TEST_ASSERT(table == NULL);

    TEST_PASS();
}

int main(void) {
    TEST_LOG("Running Object mega blaster tests");

    int failed = 0;

    failed += TestObjectScalars() != 0;
    failed += TestObjectStrings() != 0;
    failed += TestObjectCopy() != 0;
    failed += TestObjectArrayBasic() != 0;
    failed += TestObjectArrayGrowth() != 0;
    failed += TestObjectArrayCopy() != 0;
    failed += TestObjectMapBasic() != 0;
    failed += TestObjectMapTable() != 0;
    failed += TestObjectMapCopy() != 0;
    failed += TestObjectStressMixed() != 0;

    if (failed == 0) {
        TEST_LOG("ALL Object mega blaster tests passed");
        return 0;
    }

    TEST_LOG("Some Object mega blaster tests failed");
    return 1;
}
