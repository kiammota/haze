#include "server/Dispatch.h"
#include "Context.h"
#include "RawBuffer.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "api/Dispatcher.h" // <-- FALTANDO: Define DispatchResult e DispatchRPCMessage
#include "uv.h"

// Sem #ifndef aqui
DispatchBytes ServerDispatch(const Context* ctx, const uv_tcp_t *conn, RawBuffer *buffer)
{
    Request *request = RequestUnmarshal(buffer);

    if (!request)
        return (DispatchBytes){ .ok = false, .response = NULL, .notification = NULL };

    DispatchResult dr = DispatchRPCMessage(ctx, conn, request);
    RequestFree(&request);

    DispatchBytes out = { .ok = true, .response = NULL, .notification = NULL };

    if (dr.response) {
        out.response = ResponseMarshal(dr.response);
        ResponseFree(&dr.response);
    }
    if (dr.notification) {
        out.notification = NotificationMarshal(dr.notification);
        NotificationFree(&dr.notification);
    }

    return out;
}
