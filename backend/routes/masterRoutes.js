const fs = require("fs");
const express = require("express");
const router = express.Router();
const dataFile = "master_rfids.txt"; // Replace with your desired file path
const defaultName = "######";

// Function to read names from the file
function getNames() {
  try {
    const data = fs.readFileSync(dataFile, "utf8");
    return data.split(",").map((item) => item.trim()) || []; // Split by comma and trim whitespace
  } catch (err) {
    console.error("Error reading names:", err);
    return [];
  }
}

// Function to write names to the file
function writeNames(names) {
  try {
    fs.writeFileSync(dataFile, names.join(", ")); // Join with comma and space
  } catch (err) {
    console.error("Error writing names:", err);
  }
}

// Function to ensure there are always three names (with default if needed)
function ensureThreeNames(names) {
  if (names.length < 3) {
    return names.concat(Array(3 - names.length).fill(defaultName)); // Fill with defaults
  }
  return names.slice(0, 3); // Keep only the first three names
}

// GET /names - Get all names
router.get("/names", (req, res) => {
  const names = ensureThreeNames(getNames());
  res.json(names);
});

// PUT /names/:index - Update a name by index (0, 1, or 2)
router.put("/names/:index", (req, res) => {
  const index = parseInt(req.params.index);
  if (isNaN(index) || index < 0 || index >= 3) {
    return res.status(400).json({ message: "Invalid index" });
  }

  const newName = req.body.name;
  if (!newName) {
    return res.status(400).json({ message: "Name is required" });
  }

  const names = getNames();
  names[index] = newName;
  writeNames(ensureThreeNames(names)); // Ensure there are always three names

  res.json({ message: "Name updated successfully" });
});

// DELETE /names/:index - Delete a name by index (0, 1, or 2)
router.delete("/names/:index", (req, res) => {
  const index = parseInt(req.params.index);
  if (isNaN(index) || index < 0 || index >= 3) {
    return res.status(400).json({ message: "Invalid index" });
  }

  const names = getNames();
  names.splice(index, 1); // Remove the element at the specified index
  writeNames(ensureThreeNames(names)); // Ensure there are always three names

  res.json({ message: "Name deleted successfully" });
});

module.exports = router;
