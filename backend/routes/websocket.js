// const express = require("express");
// const http = require("http");
// const { Server } = require("socket.io");
// const cors = require("cors");

// const app = express();
// app.use(cors());

// const server = http.createServer(app);
// const SOCKET_IO_PORT = 5000; //dedicated port for websocket

// //creating new Server
// const io = new Server(server, {
//   cors: {
//     origin: "http://localhost:3000",
//     methods: ["GET", "POST", "DELETE", "PUT"],
//   },
// });

// //on websocket connection
// io.on("connection", async (socket) => {
//   try {
//     console.log("FrontEnd RegForm connected at port 3000 using SocketIO");

//     //Testing the Connection
//     io.emit(
//       "test-connection",
//       `"Server is listening at port ${SOCKET_IO_PORT}"`
//     );

//     socket.on("disconnect", () => {
//       console.log("RegForm disconnected");
//     });
//   } catch (error) {
//     console.log(error);
//   }
// });

// io.listen(SOCKET_IO_PORT);

//THIS IO WILL BE USED IN /admin route TO EMIT SCANNED RFID DATA
// module.exports = io;
