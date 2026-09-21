#include "Notification.h"

#include "mpack/mpack-expect.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack-writer.h"
#include "RawBuffer.h"


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NOTIFICATION_MAX_METHOD_SIZE 4096
#define NOTIFICATION_MAX_PARAMS      4096

static bool NotificationWriteObject(mpack_writer_t *writer,
                                    const Object *obj);

static Object *NotificationReadObject(mpack_reader_t *reader);
static bool NotificationWriteObject(mpack_writer_t *writer,
                                    const Object *obj)
{
    if (!writer || !obj)
        return false;

    switch (ObjectGetType(obj)) {
    case OBJ_NIL:
        mpack_write_nil(writer);
        break;

    case OBJ_BOOL:
        mpack_write_bool(writer, ObjectGetValue(obj).bool_value);
        break;

    case OBJ_INT:
        mpack_write_i64(writer, ObjectGetValue(obj).int_value);
        break;

    case OBJ_UINT:
        mpack_write_u64(writer, ObjectGetValue(obj).uint_value);
        break;

    case OBJ_FLOAT:
        mpack_write_float(writer, ObjectGetValue(obj).float_value);
        break;

    case OBJ_DOUBLE:
        mpack_write_double(writer, ObjectGetValue(obj).double_value);
        break;

    case OBJ_STR: {
        const char *str = ObjectGetStr((Object *)obj);
        size_t len = ObjectGetSize((Object *)obj);

        if (!str || len > UINT32_MAX)
            return false;

        mpack_write_str(writer, str, (uint32_t)len);
        break;
    }

    case OBJ_BIN: {
        RawBuffer *bin = ObjectGetBin((Object *)obj);

        if (!bin || bin->len > UINT32_MAX)
            return false;

        if (bin->len > 0 && !bin->data)
            return false;

        mpack_write_bin(
            writer,
            (const char *)bin->data,
            (uint32_t)bin->len
        );

        break;
    }

    case OBJ_ARRAY: {
        ObjectArray *array = ObjectGetArray((Object *)obj);

        if (!array || array->len > UINT32_MAX)
            return false;

        mpack_start_array(writer, (uint32_t)array->len);

        for (size_t i = 0; i < array->len; ++i) {
            if (!NotificationWriteObject(writer, array->objects[i]))
                return false;
        }

        mpack_finish_array(writer);
        break;
    }

    /*
     * Object atualmente não possui um campo para armazenar
     * ObjectMapTable dentro de ObjectValue.
     */
    case OBJ_MAP:
        return false;

    case OBJ_UND:
    default:
        return false;
    }

    return mpack_writer_error(writer) == mpack_ok;
}


static Object *NotificationReadObject(mpack_reader_t *reader)
{
    if (!reader)
        return NULL;

    mpack_tag_t tag = mpack_peek_tag(reader);
    mpack_type_t type = mpack_tag_type(&tag);

    switch (type) {
    case mpack_type_nil:
        mpack_expect_nil(reader);
        return ObjectCreateNil();

    case mpack_type_bool:
        return ObjectCreateBool(mpack_expect_bool(reader));

    case mpack_type_int:
        return ObjectCreateInt(mpack_expect_i64(reader));

    case mpack_type_uint:
        return ObjectCreateUInt(mpack_expect_u64(reader));

    case mpack_type_float:
        return ObjectCreateFloat(mpack_expect_float_strict(reader));

    case mpack_type_double:
        return ObjectCreateDouble(mpack_expect_double_strict(reader));

    case mpack_type_str: {
        uint32_t len = mpack_expect_str(reader);

        if (mpack_reader_error(reader) != mpack_ok)
            return NULL;

        char *buffer = malloc((size_t)len + 1);

        if (!buffer) {
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        mpack_read_bytes(reader, buffer, len);
        mpack_done_str(reader);

        if (mpack_reader_error(reader) != mpack_ok) {
            free(buffer);
            return NULL;
        }

        buffer[len] = '\0';

        /*
         * ObjectCreateStr trabalha com C string.
         * Portanto não podemos representar NUL embutido.
         */
        if (memchr(buffer, '\0', len) != NULL) {
            free(buffer);
            mpack_reader_flag_error(reader, mpack_error_type);
            return NULL;
        }

        Object *obj = ObjectNew();

        if (!obj) {
            free(buffer);
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        obj->type = OBJ_STR;
        obj->value.str_value = buffer;
        obj->size = len;

        return obj;
    }

    case mpack_type_bin: {
        uint32_t len = mpack_expect_bin(reader);

        if (mpack_reader_error(reader) != mpack_ok)
            return NULL;

        /*
         * O ObjectFree atual libera diretamente bin_value com free(),
         * então colocamos RawBuffer + seus bytes na mesma alocação.
         */
        RawBuffer *buffer = malloc(sizeof(RawBuffer) + (size_t)len);

        if (!buffer) {
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        buffer->len = len;
        buffer->data = len > 0 ? (void *)(buffer + 1) : NULL;

        if (len > 0)
            mpack_read_bytes(reader, (char *)buffer->data, len);

        mpack_done_bin(reader);

        if (mpack_reader_error(reader) != mpack_ok) {
            free(buffer);
            return NULL;
        }

        Object *obj = ObjectNew();

        if (!obj) {
            free(buffer);
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        obj->type = OBJ_BIN;
        obj->value.bin_value = buffer;
        obj->size = len;

        return obj;
    }

    case mpack_type_array: {
        uint32_t count =
            mpack_expect_array_max(reader, NOTIFICATION_MAX_PARAMS);

        if (mpack_reader_error(reader) != mpack_ok)
            return NULL;

        ObjectArray *array = ObjectArrayCreate();

        if (!array) {
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        for (uint32_t i = 0; i < count; ++i) {
            if (mpack_reader_error(reader) != mpack_ok) {
                ObjectArrayFree(&array);
                return NULL;
            }

            Object *value = NotificationReadObject(reader);

            if (!value) {
                ObjectArrayFree(&array);
                return NULL;
            }

            if (!ObjectArrayAppend(array, value)) {
                ObjectFree(&value);
                ObjectArrayFree(&array);
                mpack_reader_flag_error(reader, mpack_error_memory);
                return NULL;
            }

            ObjectFree(&value);
        }

        mpack_done_array(reader);

        if (mpack_reader_error(reader) != mpack_ok) {
            ObjectArrayFree(&array);
            return NULL;
        }

        Object *obj = ObjectNew();

        if (!obj) {
            ObjectArrayFree(&array);
            mpack_reader_flag_error(reader, mpack_error_memory);
            return NULL;
        }

        obj->type = OBJ_ARRAY;
        obj->value.array_value = array;
        obj->size = count;

        return obj;
    }

    case mpack_type_map:
        /*
         * Object não consegue representar mapa atualmente.
         */
        mpack_discard(reader);
        mpack_reader_flag_error(reader, mpack_error_unsupported);
        return NULL;

    case mpack_type_missing:
    default:
        mpack_discard(reader);
        mpack_reader_flag_error(reader, mpack_error_unsupported);
        return NULL;
    }
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

Notification *NotificationNew(void)
{
    Notification *n = malloc(sizeof(Notification));

    if (!n)
        return NULL;

    n->type = HAZE_RPC_NOTIFICATION;
    n->method = NULL;
    n->params = ObjectArrayCreate();

    if (!n->params) {
        free(n);
        return NULL;
    }

    return n;
}


void NotificationFree(Notification **n)
{
    if (!n || !*n)
        return;

    Notification *notification = *n;

    free(notification->method);

    ObjectArrayFree(&notification->params);

    free(notification);

    *n = NULL;
}


void NotificationSetMethod(Notification *n, const char *method)
{
    if (!n)
        return;

    char *copy = NULL;

    if (method) {
        copy = strdup(method);

        if (!copy)
            return;
    }

    free(n->method);
    n->method = copy;
}


RawBuffer *NotificationMarshal(Notification *n)
{
    if (!n || !n->method || !n->params)
        return NULL;

    size_t method_len = strlen(n->method);

    if (method_len > UINT32_MAX ||
        n->params->len > UINT32_MAX)
        return NULL;

    char *data = NULL;
    size_t size = 0;

    mpack_writer_t writer;

    mpack_writer_init_growable(
        &writer,
        &data,
        &size
    );

    /*
     * Notification:
     *
     * [
     *     2,
     *     "method",
     *     [...]
     * ]
     */
    mpack_start_array(&writer, 3);

    mpack_write_u64(
        &writer,
        HAZE_RPC_NOTIFICATION
    );

    mpack_write_str(
        &writer,
        n->method,
        (uint32_t)method_len
    );

    mpack_start_array(
        &writer,
        (uint32_t)n->params->len
    );

    for (size_t i = 0; i < n->params->len; ++i) {
        if (!NotificationWriteObject(
                &writer,
                n->params->objects[i])) {

            mpack_writer_flag_error(
                &writer,
                mpack_error_invalid
            );

            break;
        }
    }

    mpack_finish_array(&writer);
    mpack_finish_array(&writer);

    mpack_error_t error = mpack_writer_destroy(&writer);

    if (error != mpack_ok) {
        free(data);
        return NULL;
    }

    RawBuffer *buffer = malloc(sizeof(RawBuffer));

    if (!buffer) {
        free(data);
        return NULL;
    }

    buffer->data = data;
    buffer->len = size;

    return buffer;
}


Notification *NotificationUnmarshal(RawBuffer *b)
{
    if (!b || !b->data || b->len == 0)
        return NULL;

    mpack_reader_t reader;

    mpack_reader_init_data(
        &reader,
        (const char *)b->data,
        b->len
    );

    /*
     * Notification:
     *
     * [
     *     type,
     *     method,
     *     params
     * ]
     */
    uint32_t count =
        mpack_expect_array_max(&reader, 3);

    if (mpack_reader_error(&reader) != mpack_ok ||
        count != 3) {

        mpack_reader_destroy(&reader);
        return NULL;
    }

    mpack_expect_uint_match(
        &reader,
        HAZE_RPC_NOTIFICATION
    );

    if (mpack_reader_error(&reader) != mpack_ok) {
        mpack_reader_destroy(&reader);
        return NULL;
    }

    char *method =
        mpack_expect_cstr_alloc(
            &reader,
            NOTIFICATION_MAX_METHOD_SIZE
        );

    if (!method || mpack_reader_error(&reader) != mpack_ok) {
        free(method);
        mpack_reader_destroy(&reader);
        return NULL;
    }

    uint32_t param_count =
        mpack_expect_array_max(
            &reader,
            NOTIFICATION_MAX_PARAMS
        );

    if (mpack_reader_error(&reader) != mpack_ok) {
        free(method);
        mpack_reader_destroy(&reader);
        return NULL;
    }

    Notification *notification = NotificationNew();

    if (!notification) {
        free(method);
        mpack_reader_destroy(&reader);
        return NULL;
    }

    notification->method = method;

    for (uint32_t i = 0; i < param_count; ++i) {
        Object *obj = NotificationReadObject(&reader);

        if (!obj) {
            NotificationFree(&notification);
            mpack_reader_destroy(&reader);
            return NULL;
        }

        if (!ObjectArrayAppend(notification->params, obj)) {
            ObjectFree(&obj);
            NotificationFree(&notification);
            mpack_reader_destroy(&reader);
            return NULL;
        }

        ObjectFree(&obj);
    }

    mpack_done_array(&reader);
    mpack_done_array(&reader);

    mpack_error_t error = mpack_reader_destroy(&reader);

    if (error != mpack_ok) {
        NotificationFree(&notification);
        return NULL;
    }

    return notification;
}


Notification *NotificationCreate(
    const char *method,
    Object **params,
    size_t param_count)
{
    if (!method && param_count != 0)
        return NULL;

    if (param_count > UINT32_MAX)
        return NULL;

    Notification *notification = NotificationNew();

    if (!notification)
        return NULL;

    NotificationSetMethod(notification, method);

    if (method && !notification->method) {
        NotificationFree(&notification);
        return NULL;
    }

    for (size_t i = 0; i < param_count; ++i) {
        if (!params || !params[i]) {
            NotificationFree(&notification);
            return NULL;
        }

        if (!ObjectArrayAppend(notification->params, params[i])) {
            NotificationFree(&notification);
            return NULL;
        }
    }

    return notification;
}
