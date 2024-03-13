var express = require('express');
var router = express.Router();

const userModel = require('./users')

/* GET home page. */
router.get('/',function(req,res){
  res.render('index')
})

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
    console.error("Error finding user:", error);
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
    console.error("Error updating user:", error);
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
    console.error("Error deleting user:", error);
    res.status(500).send("Error deleting user");
  }
})



module.exports = router;
