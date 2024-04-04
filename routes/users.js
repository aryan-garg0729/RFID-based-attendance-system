const mongoose = require("mongoose");

mongoose.connect(
  process.env.DB
);
//YEH MERA PERSONAL HAI TAAKI ONLINE WALA BLAST NA HO JAE DUE TO UNCONDITIONAL CALLS
// mongoose.connect("mongodb://127.0.0.1:27017/RFID");

const studentSchema = mongoose.Schema({
  name: { type: String, required: true },
  rfid: { type: String, required: true },
  roll_no: { type: String, required: true },
  checkedIn: { type: Boolean, default: false },
  expiry_date: {
    type: Date,
    default: new Date(Date.now() + 2592000000),
  },
  entries: [
    {
      entry_type: String,
      time: {
        type: Date,
      },
    },
  ],
});

module.exports = mongoose.model("student", studentSchema);
