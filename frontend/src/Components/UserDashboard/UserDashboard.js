import React, { useEffect, useState } from "react";
import axios from "axios";
import { useLocation, useNavigate } from "react-router-dom";
import errorToast from "../Prompts/ErrorToast";
import { toast } from "react-toastify";
import ConfirmationModal from "../Prompts/ConfirmationModal";
import TableRow from "./TableRow";
import "./UserDashboard.css";
import AttendanceEntryModal from "../Prompts/AttendanceEntryModal";

const UPDATE_URL = "http://localhost:4000/admin/save";
const DELETE_URL = "http://localhost:4000/admin/delete";
const USER_DASH_URL = "http://localhost:4000/admin/findByRfid";

const UserDashboard = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const rfid = location.state?.rfid || " ";
  const defaultDetails = {
    name: "",
    rfid: "",
    roll_no: "",
    isCheckedIn: false,
    expiry_date: new Date(Date.now() + 2592000000).toISOString().slice(0, 10),
    attendance: [],
  };

  const [details, setDetails] = useState(defaultDetails);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [showUpdateModal, setShowUpdateModal] = useState(false);
  const [showAttendanceEntryModal, setAttendanceEntryModal] = useState(false);

  useEffect(() => {
    async function fetchData() {
      try {
        let dt = await axios.get(USER_DASH_URL, { params: { rfid: rfid } });
        console.log(dt.data);
        setDetails(dt.data);
      } catch (error) {
        console.error("Error fetching detail:", error);
        errorToast(error.name, error.message);
      }
    }
    fetchData();
  }, [rfid]);

  //extracting and beautifying attendance
  let attendance = details.attendance;
  attendance.sort((obj1, obj2) => {
    const date1 = new Date(obj1.date);
    const date2 = new Date(obj2.date);
    return date2 - date1; // Descending order (reversed for descending)
  });


  //prevent default form submit action
  const inputEvent = (event) => {
    let value = event.target.value;
    let name = event.target.name;

    if (name === "rfid") {
      value = value.toUpperCase();
    }
    // set details onChange event
    setDetails((previousValue) => {
      return { ...previousValue, [name]: value };
    });
  };

  const handleCloseButton = () => {
    navigate("/fetchAll");
  };

  const handleShowUpdateModal = () => {
    if (details.rfid && details.name && details.roll_no && details.expiry_date)
      setShowUpdateModal(true);
  };
  const handleShowDeleteModal = () => {
    if (details.rfid) setShowDeleteModal(true);
  };
  // close confirmation Modal
  const closeModal = () => {
    setShowDeleteModal(false);
    setShowUpdateModal(false);
  };

  //update or create student info
  const handleSaveName = async (event) => {
    try {
      if (details.rfid) {
        const res = await axios.post(UPDATE_URL, details);
        toast(`${res.status} | ${res.data.message}`);
      }
    } catch (error) {
      console.error("Error updating detail:", error);
      errorToast(error.name, error.message);
    }
  };

  //delete student
  const handleDeleteName = async (event) => {
    try {
      let res = await axios.delete(DELETE_URL, {
        data: {
          rfid: details.rfid,
        },
      });
      toast(`${res.status} | ${res.data.message}`);
    } catch (error) {
      console.error("Error deleting detail:", error);
      errorToast(error.name, error.message);
    }
  };

  const saveAttendanceEntry = async ({
    newDate,
    newCheckInTime,
    newCheckOutTime,
  }) => {
    try {
      let res = await axios.put(USER_DASH_URL, {
        rfid,
        newDate,
        newCheckInTime,
        newCheckOutTime,
      });
      toast(`${res.status} | ${res.data.message}`);
    } catch (error) {
      console.error("Error deleting detail:", error);
      errorToast(error.name, error.message);
    }
  };

  const defaultEditEntryValues = {
    rfid: rfid,
    rowData: {
      date: new Date().toISOString().slice(0, 10),
      checkIn: new Date().toISOString(),
      checkOut: new Date().toISOString(),
    },
  };
  const [editEntryValues, setEditEntryValues] = useState(
    defaultEditEntryValues
  );

  const handleEditEntry = (rowData) => {
    // console.log("defalut values ", editEntryValues);
    // console.log(rowData);
    if (Object.keys(rowData).length > 0) {
      const values = {
        rfid: rfid,
        rowData: rowData,
      };
      setEditEntryValues(values);
    }
    setAttendanceEntryModal(true);
  };
  const closeAttendanceModal = () => {
    setAttendanceEntryModal(false);
  };
  return (
    <>
      {/* Delete confirmation Modal */}
      {showDeleteModal && (
        <ConfirmationModal onCancel={closeModal} onConfirm={handleDeleteName} />
      )}
      {/* Update confirmation Modal */}
      {showUpdateModal && (
        <ConfirmationModal onCancel={closeModal} onConfirm={handleSaveName} />
      )}
      {/* Update confirmation Modal */}
      {showAttendanceEntryModal && (
        <AttendanceEntryModal
          onCancel={closeAttendanceModal}
          onConfirm={saveAttendanceEntry}
          {...editEntryValues}
        />
      )}
      <div className="dash-wrapper">
        <div className="dash-container">
          <div className="dash-header">
            <span>Student Details</span>{" "}
            <button className="close-btn" onClick={handleCloseButton}>
              X
            </button>
          </div>
          <div className="detail-section">
            <div className="dash-details">
              <div>
                <span>Name*</span>
                <input
                  type="text"
                  name="name"
                  value={details.name}
                  onChange={inputEvent}
                  required
                />
              </div>
              <div>
                <span>Roll_no*</span>
                <input
                  type="text"
                  name="roll_no"
                  value={details.roll_no}
                  onChange={inputEvent}
                  required
                />
              </div>
              <div>
                <span>RFID*</span>
                <input
                  type="text"
                  name="rfid"
                  value={details.rfid}
                  onChange={inputEvent}
                  required
                  autoCapitalize="characters"
                />
              </div>
              <div>
                <span>Expiry Date*</span>
                <input
                  type="date"
                  name="expiry_date"
                  onChange={inputEvent}
                  style={{ width: "189px" }}
                  value={details.expiry_date.slice(0, 10)}
                  required
                />
              </div>
              <div>
                <span>CheckedIn Status*</span>
                <select
                  name="checkedIn"
                  onChange={inputEvent}
                  value={details.isCheckedIn}
                  style={{ width: "189px" }}
                  required
                >
                  <option value="false">false</option>
                  <option value="true">true</option>
                </select>
              </div>
            </div>

            <div className="dash-buttons">
              <button className="btn saveBtn" onClick={handleShowUpdateModal}>
                Save
              </button>

              <button className="btn deleteBtn" onClick={handleShowDeleteModal}>
                Delete
              </button>
            </div>
          </div>
          <hr />
          <div className="attendance-section">
            <table>
              <thead>
                <tr>
                  <th>
                    <span
                      className="row-edit-btn cursor-pointer"
                      role="img"
                      aria-label="edit-button"
                      onClick={() => handleEditEntry({})}
                      title="Create New Entry"
                    >
                      ✏️
                    </span>
                    Date
                  </th>
                  <th>CheckIn Time</th>
                  <th>CheckOut Time</th>
                </tr>
              </thead>
              <tbody>
                {/* Map over the data array and render a TableRow component for each item */}
                {attendance.map((entry, index) => (
                  <TableRow
                    key={index}
                    id={index}
                    rowData={entry}
                    handleEditEntry={handleEditEntry}
                  />
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </>
  );
};

export default UserDashboard;
