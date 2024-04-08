const express = require("express");

const userModel = require("./userModel");
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

/* GET home page. */
router.get("/", function (req, res) {
  res.render("index");
});

// -----------------------Admin side MCU requests here---------------------------
// takes rfid and returns all details
let previousRfidData = {};
router.post("/admin", async function (req, res) {
  try {
    let rfid = req.body.rfid; // got the rfid
    const data = await userModel.findOne({ rfid: rfid });

    //either sending whole data or rfid only
    if (!data) {
      previousRfidData = {
        name: "",
        rfid: rfid,
        roll_no: "",
        checkedIn: "",
        expiry_date: "",
        entries: [],
      };
      res.status(404).send({ message: "user not found" });
    } else {
      previousRfidData = data;
      res.status(200).send({ message: "user found" });
    }
    //emitting searched data to frontend
    io.emit("scannedRfidData", previousRfidData);
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send("Error finding user");
  }
});

// create and update
router.post("/admin/update", async (req, res) => {
  try {
    // findandUpdate
    let { rfid, name, roll_no, checkedIn, expiry_date } = req.body;

    expiry_date = new Date(expiry_date).toISOString();
    if (!expiry_date) {
      expiry_date = new Date(Date.now() + 2592000000);
    }
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
    res.status(500).send("Error updating user");
  }
});

// delete
router.delete("/admin/delete", async (req, res) => {
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
    res.status(500).send("Error deleting user");
  }
});

// fetchAllData
router.get("/admin/fetchAll", async (req, res) => {
  try {
    let data = await userModel.find();
    if (!data) {
      res.status(404).send({ message: "Database is Empty" });
    }
    res.status(200).send(data);
  } catch (e) {
    console.error("Error getting all user:", e);
    res.status(500).send("Error getting all user");
  }
});

// fetchAll RFID AND EXPIRY DATE
router.get("/admin/fetchAllRfidAndExpiry", async (req, res) => {
  try {
    let data = await userModel.find().select("-_id rfid expiry_date");
    if (!data) {
      res.status(404).send({ message: "Database is Empty" });
    }
    res.status(200).send(data);
  } catch (e) {
    console.error("Error getting all user:", e);
    res.status(500).send("Error getting all user");
  }
});


// -----------------------Student side MCU requests here---------------------------


router.get("/allData", async (req, res) => {
  try {
    let page = req.query.page ? parseInt(req.query.page) : 1; // Get the requested page number from query parameters
    let pageSize = 100; // Define the number of items per page

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
    res.status(500).send();
  }
});

router.get("/currentDateTime", (req, res) => {
  try {
    res.status(200).send(getDateTimeObj(new Date()));
    console.log("nothing");
  } catch (error) {
    res.status(500).send({ message: error });
  }
});
// attendance h/w -> calls this route during entry and exit
router.post("/student/store", async (req, res) => {
  try {
    //MONTH IS O INDEXED
    let { rfid, year, month, date, hour, minute } = req.body;
    const checkin_date = new Date(year, month - 1, date, hour, minute);
    checkin_date.setHours(checkin_date.getHours() + 5);
    checkin_date.setMinutes(checkin_date.getMinutes() + 30);
    // console.log(checkin_date);
    let data = await userModel
      .findOne({ rfid: rfid })
      .select("-_id rfid expiry_date checkedIn");
    // console.log("Backend: " + rfid);
    // console.log(data);
    if (data == null) {
      res.status(404).send({ message: "Student not registered" });
    } else if (data.expiry_date < checkin_date) {
      res.status(403).send({ message: "Fees due" });
    } else {
      // console.log(data);
      data.checkedIn = !data.checkedIn;
      var type = data.checkedIn ? "CheckIn" : "CheckOut";
      let newEntry = { entry_type: type, time: checkin_date };
      let upd = await userModel.updateOne(
        { rfid: rfid },
        { $push: { entries: newEntry }, $set: { checkedIn: data.checkedIn } }
      );
      res.status(200).send({ message: "user found" });
    }
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send();
  }
});

module.exports = router;
