if CLIENT then

  require "socket.io"

  -- local array = { 
  --   255,
  --   255.552, 
  --   { 255, 255.552, "string", true }, 
  --   {
  --     int = 255,
  --     double = 255.552,
  --     string = "string",
  --     array = { 255, 255.552, "string" },
  --     true
  --   }, 
  --   "string" 
  -- }
  -- local object = {
  --   int = 255,
  --   double = 255.552,
  --   string = "string",
  --   array = { 255, 255.552, "string" },
  --   object = {
  --     int = 255,
  --     double = 255.552,
  --     string = "string",
  --     array = { 255, 255.552, "string" },
  --     boolean = true,
  --   },
  --   boolean = true,
  -- }

  -- local messages = {
  --   255,
  --   552.255,
  --   "string",
  --   array,
  --   object,
  --   true,
  -- }

  -- io:on("connect", function() 
  --   local recv_count = 1
  --   print "Socket connected"

  --   io:socket():on("message", function(message, need_ack, ack_messages) 
  --     PrintTable { 
  --         message = message,
  --         need_ack = need_ack,  
  --         ack_messages = ack_messages,
  --     }
  --   end)

  --   for i = 1, #messages do 
  --     io:socket():emit("message", messages[i])
  --   end 

  -- end)

  -- io:connect "ws://localhost:8080"

end