import React, { useState, useEffect } from "react";
import axios from "axios";
import "./RegForm.css";
import io from "socket.io-client";

// const GET_URL = "http://localhost:4000/admin/findByRfid";
const UPDATE_URL = "http://localhost:4000/admin/update";
const DELETE_URL = "http://localhost:4000/admin/delete";

const RegForm = () => {
  //   const [fetchedData, setFetchedData] = useState({});
  const [details, setDetails] = useState({
    _id: "",
    name: "",
    rfid: "",
    roll_no: "",
    checkedIn: "",
    expiry_date: "",
    entries: [],
  });

  const [socket, setSocket] = useState(null);

  //connecting to server
  useEffect(() => {
    const newSocket = io.connect("http://localhost:5000");
    setSocket(newSocket);

    return () => newSocket.close();
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

  // const notify = (mes) => ;
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

  const updateEvent = async (event) => {
    try {
      if (details.rfid) {
        let res = await axios.post(UPDATE_URL, details);
        window.alert(res.data.message);
      }
    } catch (e) {
      console.error(e);
      // notify("error");
    }
  };
  const deleteEvent = async (event) => {
    try {
      if (details.rfid && window.confirm("Are u confirm to delete?")) {
        // console.log(details);
        let res = await axios.delete(DELETE_URL, {
          data: {
            rfid: details.rfid,
          },
        });
        setDetails({
          _id: "",
          name: "",
          roll_no: "",
          checkedIn: "",
          expiry_date: "",
          entries: [],
        });
        window.alert(res.message);
      }
    } catch (e) {
      console.error(e);
    }
  };
  return (
    <>
      <div className="formDiv">
        <div className="form-container">
          <form onSubmit={actionSubmit}>
            <label htmlFor="rfid">RFID Tag ID</label>
            <input
              type="text"
              id="rfid"
              name="rfid"
              onChange={inputEvent}
              value={details.rfid}
            />
            <label htmlFor="name">Name</label>
            <input
              type="text"
              id="name"
              name="name"
              onChange={inputEvent}
              value={details.name}
            />
            <label htmlFor="roll_no">RollNumber</label>
            <input
              type="text"
              id="roll_no"
              name="roll_no"
              onChange={inputEvent}
              value={details.roll_no}
            />
            <label htmlFor="expiry_date">
              Validity (default one month from now)
            </label>
            <input
              type="text"
              id="expiry_date"
              name="expiry_date"
              onChange={inputEvent}
              value={details.expiry_date}
            />
            <label htmlFor="checkedIn">checkedIn</label>
            <input
              type="text"
              id="checkedIn"
              name="checkedIn"
              onChange={inputEvent}
              value={details.checkedIn}
            />
            <div className="buttonBox">
              {/* <button
              type="create"
              className="btn createButton"
              id="createButton"
            >
              Create
            </button> */}
              <button
                type="update"
                className="btn updateButton"
                id="updateButton"
                onClick={updateEvent}
              >
                Update
              </button>
              <button
                type="submit"
                className="btn deleteButton"
                id="deleteButton"
                onClick={deleteEvent}
              >
                Delete
              </button>
            </div>
          </form>
        </div>
        <div className="master-list">
          <div className="master">
            <span className={1 ? "todo-text todo-completed" : "todo-text"}>
              rfid1
            </span>
            <button>delete</button>
          </div>
          <div className="master">
            <span className={1 ? "todo-text todo-completed" : "todo-text"}>
              rfid2
            </span>
            <button>delete</button>
          </div>
          <div className="master">
            <span className={1 ? "todo-text todo-completed" : "todo-text"}>
              rfid3
            </span>
            <button>delete</button>
          </div>
        </div>
      </div>
    </>
  );
};

export default RegForm;
