var io = require('socket.io').listen(8080);

io.sockets.on('connection', function (socket) {
  console.log(`Connection from ${socket.conn.remoteAddress}`);

  socket.on('message', function (data) {
    console.log(`Client ${socket.conn.remoteAddress} emitted: `);
    console.log(JSON.stringify(data));

    socket.emit('message', data)
  });

  socket.emit('message', 255)
  socket.emit('message', 255.552)
  socket.emit('message', "string")
  socket.emit('message', [ 1, 2, 3 ])

  socket.on('disconnect', (reason) => {
    console.log(`Client ${socket.conn.remoteAddress} disconnected with reason ${reason}`);
  });
});