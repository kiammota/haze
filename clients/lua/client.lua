local socket = require("socket")
local haze = {}

haze.Client = {
  handle = nil,
  address = "",
  port_num = 0
}

haze.Client.__index = haze.Client

function haze.Client:new(address, port)
  local obj = setmetatable({}, self)

  local tcp = assert(socket.tcp())
  local success, err = tcp:connect(address, port)

  if not success then
    error("Connection with haze failed: " .. tostring(err))
  end

  obj.handle = tcp
  obj.address = address
  obj.port_num = port

  return obj
end

function haze.Client:port()
  return self.port_num
end

function haze.Client:adderss()
  return self.port_num
end

function haze.Client:send(request)
  if type(request) ~= "table" then
    print("Erro: O cliente só pode enviar tabelas!")
    return
  end

  if request.msgtype == nil or
      request.msgid == nil or
      request.method == nil or
      type(request.parameters) ~= "table" then
    print("Erro: A tabela não possui a estrutura obrigatória!")
    return
  end
  self.handle:send()

  print("Requisição válida enviada com sucesso!")
end
