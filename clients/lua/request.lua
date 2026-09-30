local haze = {}

haze.Request = {
  msgtype = 1,
  msgid = 0,
  method = "",
  parameters = {}
}

haze.Request.__index = haze.Request

function haze.Request:create(msgid, method, parameters)
  return setmetatable({
    msgtype = 1,
    msgid = msgid,
    method = "",
    parameters = {}
  }, haze.Request)
end
