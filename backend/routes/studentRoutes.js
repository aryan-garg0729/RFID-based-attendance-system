const express = require("express");

const userModel = require("../model/userModel");
const router = express.Router();
const io = require("./websocket");
const exp = require("constants");

function getDateTimeObj(arg) {
  //year month date hour minute second
  return {
    year: arg.getFullYear(),
    month: arg.getMonth(),
    date: arg.getDate(),
    hours: arg.getHours(),
    minutes: arg.getMinutes(),
    seconds: arg.getSeconds(),
  };
}
// -----------------------Student side MCU requests here---------------------------

router.get("/allData", async (req, res) => {
  try {
    let page = req.query.page ? parseInt(req.query.page) : 1; // Get the requested page number from query parameters
    let pageSize = 2; // Define the number of items per page

    // Calculate the skip value based on the requested page number and page size
    let skip = (page - 1) * pageSize;

    // Query the database to retrieve a subset of data based on pagination
    let data = await userModel
      .find()
      .skip(skip)
      .limit(pageSize)
      .select("-_id rfid expiry_date");

    // if (data.size() >= 20) morepage = true;
    // data < 20 -> false

    // console.log(data);
    if (data.length == 0) {
      return res.status(404).send({ message: "done" });
    }

    // console.log(data);
    res.status(200).send(data);
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send({message:"Server Error"});
  }
});

router.get("/currentDateTime", (req, res) => {
  try {
    res.status(200).send(getDateTimeObj(new Date()));
    console.log("nothing");
  } catch (error) {
    res.status(500).send({ message: "Server Error" });
  }
});
// attendance h/w -> calls this route during entry and exit

// POST route to handle both check-in and check-out
router.post("/store", async (req, res) => {
  try {
    const { rfid, year, month, date, hour, minute } = req.body;
    console.log(req.body);
    // Find the student by RFID
    const student = await userModel.findOne({ rfid: rfid });

    if (!student) {
      return res.status(200).json({ message: "Student not found" });
    }
    // Create a new Date object for check-in time
    const logDateTime = new Date(year, month, date, hour, minute);
    //shifting to local time
    logDateTime.setHours(logDateTime.getHours() + 5);
    logDateTime.setMinutes(logDateTime.getMinutes() + 30);
    // console.log("logDateTime", logDateTime);

    // Find today's attendance entry or create a new one
    let logDate = logDateTime.toISOString().slice(0, 10);
    // console.log("logDate ", logDate);
    //finding previous any entry
    let todayAttendance = student.attendance.find(
      (entry) => entry.date === logDate
    );

    let isUpdated = false;
    let entry_type = "";
    if (!todayAttendance) entry_type = "checkIn";
    else entry_type = "checkOut";

    if (entry_type === "checkIn") {
      //if not found any entry
      if (!todayAttendance) {
        todayAttendance = {
          date: logDate,
          checkIn: logDateTime,
        };
        student.attendance.push(todayAttendance);
        student.isCheckedIn = true;
        isUpdated = true;
      } else {
        res.status(200).json({ message: "Bad Request, can't check In" });
      }
    } else if (entry_type === "checkOut") {
      if (todayAttendance) {
        todayAttendance.checkOut = logDateTime;
        student.isCheckedIn = false;
        isUpdated = true;
      } else {
        res.status(200).json({ message: "Bad Request, can't check Out" });
      }
    } else {
      return res.status(200).json({ message: "Invalid entry type" });
    }

    if (isUpdated) {
      await student.save();
      res.status(200).json({ message: "Entry added successfully" });
    }
  } catch (error) {
    console.error("Error processing request:", error);
    res.status(500).json({ message: "Server error" });
  }
});

module.exports = router;
