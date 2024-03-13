var express = require('express');
var router = express.Router();

const userModel = require('./users')

/* GET home page. */
router.get('/',function(req,res){
  res.render('index')
})


// -----------------------Admin side MCU requests here---------------------------

// takes rfid and returns all details
router.post('/admin',async function(req,res){
  try {
    let rfid = req.body.rfid;
    let data = await userModel.findOne({rfid : rfid})
    
    if(data==null){
      data={"rfid":rfid}
    }
    console.log(data)
    res.send(data);
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send("Error finding user");
  }
  
})

// create
router.post('/admin/create', async function(req, res) {
  try {
    // Create a new user using the userModel
    let data = await userModel.create({
      name: req.body.name,
      rfid: req.body.rfid,
      roll_no: req.body.roll,
      checkedIn: false,
      expiry_date:req.body.expiry_date
    });
    // Send back the created user object as a response
    res.send(data);
  } catch (error) {
    // Handle any errors that occur during user creation
    console.error("Error creating user:", error);
    res.status(500).send("Error creating user");
  }
});


// update
router.post("/admin/update",async(req,res)=>{
  try{
      let rfid = req.body.rfid;
      let data = await userModel.updateOne({rfid:rfid},{$set:{
        name: req.body.name,
        roll_no: req.body.roll_no,
        checkedIn: req.body.checkedIn,
        expiry_date:req.body.expiry_date
      }})
      res.send(data);
  }
  catch(e){
    console.error("Error updating user:", e);
    res.status(500).send("Error updating user");
  }
})


// delete
router.post("/admin/delete",async(req,res)=>{
  try{
      let rfid = req.body.rfid;
      let data = await userModel.deleteOne({ rfid: rfid });
      res.send(data);
  }
  catch(e){
    console.error("Error deleting user:", e);
    res.status(500).send("Error deleting user");
  }
})

// -----------------------Student side MCU requests here---------------------------

router.post("/student",async(req,res)=>{
  try {
    let rfid = req.body.rfid;
    let data = await userModel.findOne({rfid : rfid})
    
    if(data==null){
      res.status(404).send();
    }
    else if(data.expiry_date<Date.now()){
      res.status(403).send();
    }
    else{
      console.log(data)
      data.checkedIn = !data.checkedIn;
      var type = data.checkedIn?"CheckIn":"CheckOut";
      let newEntry = {entry_type:type,time:req.body.time}
      let upd = await userModel.updateOne({rfid:rfid},{$push:{entries:newEntry},$set:{checkedIn:data.checkedIn}})
      res.status(200).send(upd);
    }
  } catch (e) {
    console.error("Error finding user:", e);
    res.status(500).send();
  }
})

module.exports = router;
