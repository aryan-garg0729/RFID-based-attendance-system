const express = require("express");

const userModel = require("../model/userModel");
const router = express.Router();
const io = require("../app.js");
const exp = require("constants");

// /* GET home page. */
// router.get("/", function (req, res) {
//   res.render("index");
// });

// -----------------------Admin side MCU requests here---------------------------
// takes rfid from nodeMCU and returns all details to frontend regForm
let previousRfidData = {};
router.post("/", async function (req, res) {
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

// create and update in same route
router.post("/save", async (req, res) => {
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
router.delete("/delete", async (req, res) => {
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
router.get("/fetchAll", async (req, res) => {
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
router.get("/fetchAllRfidAndExpiry", async (req, res) => {
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
router.get("/findByRfid/", async (req, res) => {
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

router.put("/findByRfid", async (req, res) => {
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
module.exports = router;
