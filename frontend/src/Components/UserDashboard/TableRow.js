import React, { useState } from "react";

const TableRow = ({ rowData, id, handleEditEntry }) => {
  // Destructure rowData to access individual properties
  let { date, checkIn, checkOut, throughAdmin } = rowData;

  const [showEditButton, setShowEditButton] = useState(true);
  checkIn = checkIn.slice(11, 19);
  if (checkOut) checkOut = checkOut.slice(11, 19);
  else checkOut = "- - ";

  return (
    <tr
      className="attendance-row"
      onMouseEnter={() => setShowEditButton(id)}
      onMouseLeave={() => setShowEditButton(null)}
    >
      <td>
        {showEditButton === id ? (
          <span
            className="row-edit-btn cursor-pointer"
            role="img"
            aria-label="edit-button"
            onClick={() => handleEditEntry(rowData)}
            title="Edit Entry"
          >
            ✏️
          </span>
        ) : (
          <span>&nbsp;&nbsp;&nbsp;&nbsp;</span>
        )}
        {date}
        {throughAdmin ? (
          <sup id="throughAdmin" title="Entry Saved Through Admin">
            ##
          </sup>
        ) : (
          " "
        )}
      </td>
      <td>{checkIn}</td>
      <td>{checkOut}</td>
    </tr>
  );
};

export default TableRow;
