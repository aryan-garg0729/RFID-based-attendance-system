const mongoose = require("mongoose");

// mongoose.connect(
//   process.env.DB
// );
//YEH MERA PERSONAL HAI TAAKI ONLINE WALA BLAST NA HO JAE DUE TO UNCONDITIONAL CALLS
mongoose.connect("mongodb://127.0.0.1:27017/RFID2");

const studentSchema = mongoose.Schema({
  name: { type: String, required: true },
  rfid: { type: String, required: true, unique: true },
  roll_no: { type: String, required: true, unique: true },
  isCheckedIn: { type: Boolean, default: false },
  expiry_date: {
    type: Date,
    default: new Date(Date.now() + 2592000000),
  },
  attendance: [
    {
      throughAdmin: { type: Boolean, default: false },
      date: {
        type: String,
        required: true,
      },
      checkIn: { type: Date, default: Date.now },
      checkOut: { type: Date, default: null },
    },
  ],
});

module.exports = mongoose.model("student", studentSchema);
