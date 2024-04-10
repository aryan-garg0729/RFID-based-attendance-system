import React, { useState } from "react";
import axios from "axios";
import { useLocation, useNavigate } from "react-router-dom";
import errorToast from "../Prompts/ErrorToast";
import { toast } from "react-toastify";
import ConfirmationModal from "../Prompts/ConfirmationModal";
import "./UserDashboard.css";

const UPDATE_URL = "http://localhost:4000/admin/update";
const DELETE_URL = "http://localhost:4000/admin/delete";

const UserDashboard = (props) => {
  const navigate = useNavigate();
  const location = useLocation();
  const rowData = location.state?.rowData || {};

  // const defaultDetails = {
  //   _id: "",
  //   name: "",
  //   rfid: "",
  //   roll_no: "",
  //   checkedIn: false,
  //   expiry_date: new Date(Date.now() + 2592000000).toISOString().slice(0, 10),
  //   entries: [],
  // };

  const [details, setDetails] = useState(rowData);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [showUpdateModal, setShowUpdateModal] = useState(false);

  //prevent default form submit action
  const inputEvent = (event) => {
    let value = event.target.value;
    let name = event.target.name;

    // set details onChange event
    setDetails((previousValue) => {
      return { ...previousValue, [name]: value };
    });
  };

  const handleCloseButton = () => {
    navigate("/fetchAll");
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
                <span>Name</span>
                <input
                  type="text"
                  name="name"
                  value={details.name}
                  onChange={inputEvent}
                />
              </div>
              <div>
                <span>Roll_no</span>
                <input
                  type="text"
                  name="roll_no"
                  value={details.roll_no}
                  onChange={inputEvent}
                />
              </div>
              <div>
                <span>RFID</span>
                <input
                  type="text"
                  name="rfid"
                  value={details.rfid}
                  onChange={inputEvent}
                  autoCapitalize="characters"
                />
              </div>
              <div>
                <span>Expiry Date</span>
                <input
                  type="date"
                  name="expiry_date"
                  onChange={inputEvent}
                  style={{ width: "189px" }}
                  value={details.expiry_date.slice(0, 10)}
                />
              </div>
              <div>
                <span>CheckedIn Status</span>
                <select
                  name="checkedIn"
                  onChange={inputEvent}
                  value={details.checkedIn}
                  style={{ width: "189px" }}
                >
                  <option value="false">false</option>
                  <option value="true">true</option>
                </select>
              </div>
            </div>

            <div className="dash-buttons">
              <button className="btn saveBtn" onClick={() => setShowUpdateModal(true)}>
                Save
              </button>

              <button className="btn deleteBtn" onClick={() => setShowDeleteModal(true)}>
                Delete
              </button>
            </div>
          </div>
          <hr />
          <div className="entry-section">
            <div className="checkIn">
              <span>checkIn</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkIn">
              <span>checkIn</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkIn">
              <span>checkIn</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
            <div className="checkOut">
              <span>checkOut</span>
              <span>Time:</span>
            </div>
          </div>
        </div>
      </div>
    </>
  );
};

export default UserDashboard;
