var createError = require("http-errors");
var express = require("express");
var path = require("path");
var cookieParser = require("cookie-parser");
var logger = require("morgan");
var cors = require("cors");
const { Server } = require("socket.io");
const dotenv = require("dotenv");
dotenv.config({ path: "./.env" });
var http = require("http");
const adminRouter = require("./routes/adminRoutes");
const studentRouter = require("./routes/studentRoutes");
const masterRouter = require("./routes/masterRoutes");

var app = express();
app.use(cors());

// view engine setup
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "ejs");

app.use(logger("dev"));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, "public")));

app.use("/admin", adminRouter);
app.use("/student", studentRouter);
app.use("/admin/master", masterRouter);

// catch 404 and forward to error handler
app.use(function (req, res, next) {
  next(createError(404));
});

// error handler
app.use(function (err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get("env") === "development" ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render("error");
});

var port = normalizePort(process.env.PORT || "4000");
app.set("port", port);
module.exports = app;
var server = http.createServer(app);

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
      `"Server is listening at port"`
    );

    socket.on("disconnect", () => {
      console.log("RegForm disconnected");
    });
  } catch (error) {
    console.log(error);
  }
});

// io.listen(SOCKET_IO_PORT);

//THIS IO WILL BE USED IN /admin route TO EMIT SCANNED RFID DATA
module.exports = {io, server, app};