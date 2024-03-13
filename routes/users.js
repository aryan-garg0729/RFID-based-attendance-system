const mongoose = require("mongoose");

mongoose.connect("mongodb+srv://aakshatmalhotra100:4FMJMGLHK1vMRKMA@cluster0.kqekdee.mongodb.net/")

const studentSchema = mongoose.Schema({
  name:String,
  rfid:String,
  roll_no:String,
  checkedIn:Boolean,
  entries:[
    {
      entry_type:String,
      time: {
        type: Date
      }
    }
  ],
  expiry_date:{
    type:Date,
    default: Date.now()+2592000000
  },
})

module.exports = mongoose.model("student",studentSchema);

/*
convert date.now() to prettier form

const timestamp = Date.now();
const date = new Date(timestamp);

// Convert to a more readable format
const dateString = date.toLocaleString(); // Full date and time
// Or you can use specific methods for date and time:
// const dateString = date.toLocaleDateString(); // Date only
// const timeString = date.toLocaleTimeString(); // Time only

console.log(dateString);
*/

