import React, { useState, useEffect } from "react";
import axios from "axios";
import errorToast from "../Prompts/ErrorToast";
import { toast } from "react-toastify";
import MasterCardSection from "./MasterSection/MasterCardSection";
import io from "socket.io-client";
import "./RegForm.css";
import ConfirmationModal from "../Prompts/ConfirmationModal";

const UPDATE_URL = "http://localhost:4000/admin/save";
const DELETE_URL = "http://localhost:4000/admin/delete";

const RegForm = () => {
  //   const [fetchedData, setFetchedData] = useState({});
  const defaultDetails = {
    _id: "",
    name: "",
    rfid: "",
    roll_no: "",
    checkedIn: false,
    expiry_date: new Date(Date.now() + 2592000000).toISOString(),
    entries: [],
  };

  const [details, setDetails] = useState(defaultDetails);
  const [socket, setSocket] = useState(null);
  const [showModal, setShowModal] = useState(false);
  //connecting to server
  useEffect(() => {
    try {
      const newSocket = io.connect("http://localhost:5000");
      setSocket(newSocket);

      return () => newSocket.close();
    } catch (error) {
      console.error("Error connecting server:", error);
      errorToast(error.name, error.message);
    }
  }, []);

  //listening to server
  useEffect(() => {
    if (socket) {
      //Testing the Connection
      socket.on("test-connection", (message) => {
        console.log(`SocketIO connection established`);
        console.log(message);
      });

      socket.on("scannedRfidData", (data) => {
        console.log("scannedRfidData Received from server:", data);
        setDetails(data);
      });
    }
  }, [socket]);

  const handleShowModal = () => {
    if (details.name && details.roll_no && details.rfid) setShowModal(true);
  };
  // close confirmation Modal
  const closeModal = () => setShowModal(false);

  //prevent default form submit action
  const inputEvent = (event) => {
    let value = event.target.value;
    let name = event.target.name;

    // set details onChange event
    setDetails((previousValue) => {
      return { ...previousValue, [name]: value };
    });
  };

  //form's default action webpage is refreshing the webpage hence we lost the user value
  const actionSubmit = (event) => {
    event.preventDefault();
    // console.log(details);
  };

  //update or create student info
  const updateEvent = async (event) => {
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
  const deleteEvent = async (event) => {
    try {
      let res = await axios.delete(DELETE_URL, {
        data: {
          rfid: details.rfid,
        },
      });
      setDetails(defaultDetails);
      toast(`${res.status} | ${res.data.message}`);
    } catch (error) {
      console.error("Error deleting detail:", error);
      errorToast(error.name, error.message);
    }
  };
  return (
    <>
      {/* confirmation Modal */}
      {showModal && details.name && details.roll_no && details.rfid && (
        <ConfirmationModal onCancel={closeModal} onConfirm={deleteEvent} />
      )}
      {/* MASTERS RFID SECTION */}
      <MasterCardSection />
      {/* REGISTRATION FORM */}
      <div className="formDiv">
        <div className="form-container">
          <form onSubmit={actionSubmit}>
            <label htmlFor="rfid">
              RFID Tag ID
              <span id="red-star" title="mandatory">
                *
              </span>
            </label>
            <input
              type="text"
              id="rfid"
              name="rfid"
              className="form-input-style focus-bck hover-bck-gray"
              onChange={inputEvent}
              value={details.rfid}
              required
            />
            <label htmlFor="name">
              Name
              <span id="red-star" title="mandatory">
                *
              </span>
            </label>
            <input
              type="text"
              id="name"
              name="name"
              className="form-input-style focus-bck hover-bck-gray"
              onChange={inputEvent}
              value={details.name}
              required
            />
            <label htmlFor="roll_no">
              RollNumber
              <span id="red-star" title="mandatory">
                *
              </span>
            </label>
            <input
              type="text"
              id="roll_no"
              name="roll_no"
              className="form-input-style focus-bck hover-bck-gray"
              onChange={inputEvent}
              value={details.roll_no}
              required
            />
            <label htmlFor="expiry_date">
              Validity (default one month from now)
            </label>
            <input
              type="date"
              id="expiry_date"
              name="expiry_date"
              className="form-input-style focus-bck hover-bck-gray"
              onChange={inputEvent}
              value={details.expiry_date.slice(0, 10)}
              required
            />
            <label htmlFor="checkedIn">checkedIn</label>
            <select
              id="checkedIn"
              name="checkedIn"
              className="form-input-style focus-bck hover-bck-gray"
              onChange={inputEvent}
              value={details.checkedIn}
              required
            >
              <option value="false">false</option>
              <option value="true">true</option>
            </select>
            <div className="buttonBox">
             
              <button
                type="submit"
                className="btn saveBtn"
                id="updateButton"
                onClick={updateEvent}
                title="Update/Create"
              >
                Save
              </button>
              <button
                type="submit"
                className="btn deleteBtn"
                id="deleteButton"
                onClick={handleShowModal}
                title="Delete"
              >
                Delete
              </button>
            </div>
          </form>
        </div>
      </div>
    </>
  );
};

export default RegForm;
