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
const userModel = require("./model/userModel");

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

app.get('/', (req, res)=> {
  res.json("Hello");
})
let previousRfidData = {};
app.post("/admin", async function (req, res) {
  try {
    let rfid = req.body.rfid; // got the rfid
    console.log("rfid" , rfid);
    const data = await userModel.findOne({ rfid: rfid });

    //either sending whole data or rfid only
    console.log("data : ", data);
    if (!data) {
      previousRfidData = {
        name: "",
        rfid: rfid,
        roll_no: "",
        checkedIn: "",
        expiry_date: new Date(Date.now() + 2592000000).toISOString(),
        attendance: [],
      };
      res.status(404).send({ message: "user not found" });
    } else {
      previousRfidData = data;
      res.status(200).send({ message: "user found", data });
    }
    //emitting searched data to frontend
    io.emit("scannedRfidData", previousRfidData);
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send("Error finding user");
  }
});

/// *****
// create and update in same route
app.post("admin/save", async (req, res) => {
  try {
    console.log(req.body);
    // findandUpdate
    let { rfid, name, roll_no, checkedIn, expiry_date } = req.body;

    if (!expiry_date) {
      expiry_date = new Date(Date.now() + 2592000000);
    }
    expiry_date = new Date(expiry_date).toISOString();
    const std = await userModel.findOne({ rfid: rfid });
    //if user not found then create a new user
    if (!std) {
      let data = await userModel.create({
        rfid,
        name,
        roll_no,
        checkedIn,
        expiry_date,
      });
      // Send back the created user object as a response
      res.status(201).send({ message: "New User Created Successfully", data });
    } else {
      //else update the existing user
      let data = await userModel.updateOne(
        { rfid: rfid },
        {
          $set: {
            name: name,
            roll_no: roll_no,
            checkedIn: checkedIn,
            expiry_date: expiry_date,
          },
        }
      );
      res.status(200).send({ message: "User Updated Successfully", data });
    }
  } catch (e) {
    console.error("Error updating user:", e);
    res.status(500).send({message:"Error updating user"});
  }
});

// delete
app.delete("admin/delete", async (req, res) => {
  try {
    let rfid = req.body.rfid;
    let data = await userModel.findOne({ rfid: rfid });

    if (!data) {
      res.status(404).send({ message: "User doesn't exist" });
    } else {
      data = await userModel.findOneAndDelete({ rfid: rfid });
      res.status(200).send({ message: "User Deleted Successfully", data });
    }
  } catch (e) {
    console.error("Error deleting user:", e);
    res.status(500).send({message:"Error deleting user"});
  }
});

// fetchAllData
app.get("admin/fetchAll", async (req, res) => {
  try {
    let data = await userModel
      .find()
      .select("-_id rfid roll_no name isCheckedIn");
    if (!data) {
      res.status(404).send({ message: "Database is Empty" });
    }
    res.status(200).send(data);
  } catch (e) {
    console.error("Error getting all user:", e);
    res.status(500).send({message:"Error getting all user"});
  }
});

// fetchAll RFID AND EXPIRY DATE
app.get("admin/fetchAllRfidAndExpiry", async (req, res) => {
  try {
    let data = await userModel.find().select("-_id rfid expiry_date");
    if (!data) {
      res.status(404).send({ message: "Database is Empty" });
    }
    res.status(200).send(data);
  } catch (e) {
    console.error("Error getting all user:", e);
    res.status(500).send({message:"Error getting all user"});
  }
});

//------------------USER DAHSBOARD ROUTES--------------------//
app.get("admin/findByRfid/", async (req, res) => {
  try {
    const rfid = req.query.rfid;
    console.log("rfid ", rfid);
    const data = await userModel.findOne({ rfid: rfid }).select("-_id -__v");
    if (data) {
      res.status(200).send(data);
    } else {
      res.status(404).send({ message: "student not found" });
    }
  } catch (error) {
    console.error("Error finding user:", e);
    res.status(500).send({ message: "Error finding user" });
  }
});

app.put("admin/findByRfid", async (req, res) => {
  try {
    let { rfid, newDate, newCheckInTime, newCheckOutTime } = req.body;
    // Find the student by RFID
    const student = await userModel.findOne({ rfid: rfid });
    newCheckInTime = parseDateTime(newDate, newCheckInTime);
    newCheckOutTime = parseDateTime(newDate, newCheckOutTime);

    //finding previous any entry
    let todayAttendance = student.attendance.find(
      (entry) => entry.date === newDate
    );
    //if not found any entry
    if (!todayAttendance) {
      todayAttendance = {
        date: newDate,
        checkIn: newCheckInTime,
        checkOut: newCheckOutTime,
        throughAdmin : true
      };
      student.attendance.push(todayAttendance);
    } else {
      todayAttendance.checkIn = newCheckInTime;
      todayAttendance.checkOut = newCheckOutTime;
      todayAttendance.throughAdmin = true;
    }
    await student.save();
    res.status(200).json({ message: "Entry Added successfully" });
  } catch (error) {
    console.error("Error finding user:", e);
    res.status(500).send({ message: "Error finding user" });
  }
});

// Function to parse date and time strings and create Date objects
const parseDateTime = (dateString, timeString) => {
  // Split the date string into year, month, and day parts
  const [year, month, day] = dateString.split("-").map(Number);

  // Split the time string into hours and minutes parts
  const [hours, minutes] = timeString.split(":").map(Number);

  // Return a new Date object with the parsed values
  let d = new Date(year, month - 1, day, hours, minutes);
  d.setHours(d.getHours() + 5);
  d.setMinutes(d.getMinutes() + 30);
  return d;
};



//// **** 





// app.use("/admin", adminRouter);
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