#!/usr/bin/env node

/**
 * Module dependencies.
 */

var app = require("../app");
var debug = require("debug")("rfid-iot:server");
var http = require("http");
const socketIO = require("socket.io");
const { Server } = require("socket.io");

/**
 * Get port from environment and store in Express.
 */

var port = normalizePort(process.env.PORT || "4000");
app.set("port", port);

/**
 * Create HTTP server.
 */

var server = http.createServer(app);

/**
 * Listen on provided port, on all network interfaces.
 */

// const io = socketIO(server);
const io = new Server(server, {
  cors: {
    origin: "http://localhost:3000",
    methods: ["GET", "POST", "DELETE", "PUT"],
  },
});

io.on("connection", async (socket) => {
  try {
    console.log("FrontEnd RegForm connected at port 3000 using SocketIO");

    //Testing the Connection
    io.emit(
      "test-connection",
      `"hello brother"`
    );

    io.emit("sample", "bhai");
    socket.on("disconnect", () => {
      console.log("RegForm disconnected");
    });
    
  } catch (error) {
    console.log(error);
  }
});




// io.on("connection",  (socket) => {
//   console.log("WebSocket connected");
//   io.emit(
//     "test-connection",
//     `"hello brother"`
//   );

//   socket.on("message", (message) => {
//     console.log("received: %s", message);
//   });

//   socket.on("disconnect", () => {
//     console.log("WebSocket disconnected");
//   });
// });


server.listen(port, ()=>{
  console.log(`Server running on PORT:${port}`)
});
server.on("error", onError);
server.on("listening", onListening);

/**
 * Normalize a port into a number, string, or false.
 */

function normalizePort(val) {
  var port = parseInt(val, 10);

  if (isNaN(port)) {
    // named pipe
    return val;
  }

  if (port >= 0) {
    // port number
    return port;
  }

  return false;
}

/**
 * Event listener for HTTP server "error" event.
 */

function onError(error) {
  if (error.syscall !== "listen") {
    throw error;
  }

  var bind = typeof port === "string" ? "Pipe " + port : "Port " + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
    case "EACCES":
      console.error(bind + " requires elevated privileges");
      process.exit(1);
      break;
    case "EADDRINUSE":
      console.error(bind + " is already in use");
      process.exit(1);
      break;
    default:
      throw error;
  }
}

/**
 * Event listener for HTTP server "listening" event.
 */

function onListening() {
  var addr = server.address();
  var bind = typeof addr === "string" ? "pipe " + addr : "port " + addr.port;
  debug("Listening on " + bind);
}

module.exports = io;
