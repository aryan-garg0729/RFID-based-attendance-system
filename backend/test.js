const express = require("express");
const http = require("http");
const { Server } = require("socket.io");
const cors = require("cors");

const app = express();
const server = http.createServer(app);
const io = new Server(server);

const PORT = 4000;

// Start listening after setting up everything
server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
  });
  

// Socket.IO event handlers
io.on("connection", (socket) => {
  console.log("A user connected");
  socket.on("disconnect", () => {
    console.log("User disconnected");
  });
});

// Express middleware for CORS
// app.use(cors({
//   origin: "*"
// }));

// Express route


// Export the io object
module.exports = {io, app};
