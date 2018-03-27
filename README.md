# gm_socket.io

An in-progress glua binding for the cpp-socket.io library.

## Documentation

### Objects
```lua
-- Creates new socket client
-- @return table
io.Client() 

-- Bind a callback to the specified event name
-- @param string    event_name 
-- @param function  callback 
Client:on(event_name, callback)

-- Returns a socket object with specified namespace
-- @param string?   nsp
-- @return Socket
Client:socket(nsp)

-- Connects client to specified host/port
-- @param string  host
-- @param number  port
Client:connect(host, port)

-- Disconnects client
Client:disconnect()

-- Returns true if client is connected
-- @return boolean
Client:is_connect()

-- Bind a callback to the specified event name
-- @param string    event_name
-- @param function  callback
Socket:on(event_name, callback)

-- Removes callbacks for specified event name
-- @param string  event_name
Socket:off(event_name)

-- Removes all callbacks for all events
Socket:off_all()

-- Returns socket namespace
-- @return string
Socket:get_namespace()
```
### Events
```lua
--[[
  Client
    "error" 
    "connect"
    "disconnect"
    "reconnect"
    "reconnecting"
```
### Callbacks

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Bromvlieg for the Engine idea
* The peeps who made [LuaInterface](https://github.com/glua/LuaInterface) and [socket.io-client-cpp](https://github.com/socketio/socket.io-client-cpp)
* [Kerrek SB](https://stackoverflow.com/users/596781/kerrek-sb) for his [ninja-like C+11 template skills](https://stackoverflow.com/a/10766422)
