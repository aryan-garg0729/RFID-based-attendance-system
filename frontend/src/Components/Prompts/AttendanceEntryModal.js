import React, { useEffect, useState } from "react";
import ReactDOM from "react-dom";
import "./AttendanceEntryModal.css";
import ConfirmationModal from "./ConfirmationModal";

const AttendanceEntryModal = ({ onCancel, onConfirm, rfid, rowData }) => {
  const [showModal, setShowModal] = useState(false);
  let { date, checkIn, checkOut } = rowData;
  checkIn = checkIn.slice(11, 16);
  checkOut = checkOut.slice(11, 16);
  const [details, setDetails] = useState({
    newDate: date,
    newCheckInTime: checkIn,
    newCheckOutTime: checkOut,
  });

  // close confirmation Modal
  const closeModal = () => setShowModal(false);

  const handleShowConfirmationModal = () => {
    setShowModal(true);
  };

  //prevent default form submit action
  const inputEvent = (event) => {
    let value = event.target.value;
    let name = event.target.name;

    // set details onChange event
    setDetails((previousValue) => {
      return { ...previousValue, [name]: value };
    });
  };

  useEffect(() => {
    document.body.style.overflowY = "hidden";
    return () => {
      document.body.style.overflowY = "scroll";
    };
  });
  return ReactDOM.createPortal(
    <>
      {showModal && (
        <ConfirmationModal onCancel={closeModal} onConfirm={() => onConfirm({ ...details })} />
      )}
      <div className="modal-wrapper">
        <div className="modal-container">
          <div className="edit-attendance-form">
            <h6>Update or Create new Attendance Entry</h6>
            <h6>For Rfid : {rfid}</h6>
            <div>
              <label htmlFor="newDate">Select Date</label>
              <input
                id="newDate"
                type="date"
                value={details.newDate}
                onChange={inputEvent}
                name="newDate"
                required
              />
            </div>
            <div>
              <label htmlFor="checkInTime">Select checkIn time:</label>
              <input
                type="time"
                id="checkInTime"
                name="newCheckInTime"
                value={details.newCheckInTime}
                onChange={inputEvent}
                step="3600"
                min="00:00"
                max="23:59"
                pattern="[0-2][0-9]:[0-5][0-9]"
                required
              ></input>
            </div>
            <div>
              <label htmlFor="checkOutTime">Select checkOut time:</label>
              <input
                type="time"
                id="checkOutTime"
                name="newCheckOutTime"
                value={details.newCheckOutTime}
                onChange={inputEvent}
                step="3600"
                min="00:00"
                max="23:59"
                pattern="[0-2][0-9]:[0-5][0-9]"
                required
              ></input>
            </div>
          </div>
          <div className="modal-button-box">
            <button className="btn" onClick={onCancel} autoFocus>
              Cancel
            </button>
            <button
              className="btn saveBtn"
              onClick={handleShowConfirmationModal}
            >
              Save
            </button>
          </div>
        </div>
      </div>
    </>,
    document.getElementById("modalRoot")
  );
};

export default AttendanceEntryModal;
