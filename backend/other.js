const {app, io} = require("./test.js");

// Now you can use the io object here to emit events or perform other Socket.IO operations
// io.emit("custom-event", "This is a custom event sent from another file");
app.get("/test", (req, res) => {
    // Emit a test event to all connected clients
    io.emit("test-event", "This is a test event from the backend");
  
    res.send("Test event emitted to Socket.IO server");
  });
  