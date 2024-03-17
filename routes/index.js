var express = require("express");
var router = express.Router();

const userModel = require("./users");

/* GET home page. */
router.get("/", function (req, res) {
  res.render("index");
});

// -----------------------Admin side MCU requests here---------------------------

// takes rfid and returns details by rfid
router.get("/admin/findByRfid", async function (req, res) {
  try {
    let rfid = req.body.rfid; // got the rfid
    let data = await userModel.findOne({ rfid: rfid });

    if (!data) {
      res.status(404).send({ message: "user not found", rfid: rfid }); //
    }
    res.status(200).send(data); // pura data hoga ya fir only rfid
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send("Error finding user");
  }
});

// frontend ke paas ab rfid aa gyi
// update and insert new student kar sakta using this

// create -> student create
router.post("/admin/create", async function (req, res) {
  try {
    // Create a new user using the userModel
    const { rfid } = req.body;
    const std = await userModel.findOne({ rfid: rfid });
    if (std) {
      return res.status(409).send({ message: "user already exists" });
    }

    let data = await userModel.create(req.body);
    // Send back the created user object as a response
    res.status(201).send({ message: "User Created Successfully", data });
  } catch (error) {
    // Handle any errors that occur during user creation
    console.error("Error creating user:", error);
    res.status(500).send("Error creating user");
  }
});

// update
router.post("/admin/update", async (req, res) => {
  try {
    // findandUpdate
    let rfid = req.body.rfid;
    // const std = await userModel.findOne({rfid: rfid});
    // if (!std){
    //   res.status(404).send({message : "user doesn't exist"});
    // }
    let data = await userModel.updateOne(
      { rfid: rfid },
      {
        $set: {
          name: req.body.name,
          roll_no: req.body.roll_no,
          checkedIn: req.body.checkedIn,
          expiry_date: req.body.expiry_date,
        },
      }
    );
    res.send(data);
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

// -----------------------Student side MCU requests here---------------------------
// router.get('/allData', async (req, res) =>{
//   try{
//     let data = await userModel.find();
//     console.log(data);
//     let sendData = [];
//     for (let i = 0; i < data.length; i++){

//       sendData.push({"rfid": data[i].rfid, "expiry_date": data[i].expiry_date})
//     }

//     res.status(200).send(sendData);
//   }
//   catch (e) {
//     console.error("Error finding user:", e);
//     res.status(500).send();
//   }
// })

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

    console.log(data);
    if (data.length == 0) {
      return res.status(404).send({ message: "done" });
    }
    // let sendData = [];
    // for (let i = 0; i < data.length; i++) {
    //   sendData.push({"rfid": data[i].rfid, "expiry_date": data[i].expiry_date});
    // }

    res.status(200).send(data);
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send();
  }
});

// attendance h/w -> calls this route during entry and exit
router.post("/student", async (req, res) => {
  try {
    let { rfid, time } = req.body;
    let data = await userModel
      .findOne({ rfid: rfid })
      .select("-_id rfid expiry_date checkedIn");

    if (data == null) {
      res.status(404).send({ message: "Student not registered" });
    } else if (data.expiry_date < Date.parse(time)) {
      res.status(403).send({ message: "Fees due" });
    } else {
      console.log(data);
      data.checkedIn = !data.checkedIn;
      var type = data.checkedIn ? "CheckIn" : "CheckOut";
      let newEntry = { entry_type: type, time: time };
      let upd = await userModel.updateOne(
        { rfid: rfid },
        { $push: { entries: newEntry }, $set: { checkedIn: data.checkedIn } }
      );
      res.status(200).send(upd);
    }
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send();
  }
});

module.exports = router;
