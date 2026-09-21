#ifndef HAZE_SERVER_DISPATCHER
#define HAZE_SERVER_DISPATCHER

#include "server/Dispatch.h"
#include "Context.h"
#include "RawBuffer.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "uv.h"

DispatchBytes HazeServerAPIDispatcher(const Context* ctx, uv_tcp_t *conn, RawBuffer *buffer)
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

#endif
