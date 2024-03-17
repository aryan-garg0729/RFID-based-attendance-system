import React, { useState, useEffect } from "react";
import axios from "axios";
import "./RegForm.css";

const GET_URL = "http://localhost:4000/admin/findByRfid";
const UPDATE_URL = "http://localhost:4000/admin/update";
// const DELETE_URL = "http://localhost:4000/admin/delete";

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

  useEffect(() => {
    // fetchData() will run every sec;
    console.log("running useEffect");
    const interval = setInterval(fetchData, 2000);
    return () => {
      clearInterval(interval);
    };
  }, []);
  //fetch data from backend
  async function fetchData() {
    try {
      const result = await axios(GET_URL, { params: { rfid: "" } });

      // if there is rfid present
      if (result.data.rfid) setDetails(result.data);
    } catch (e) {
      console.log(e);
    }
  }
  console.log(details);

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
    console.log(details);
  };

  const updateEvent = async (event) => {
    try {
      const res = await axios.post(UPDATE_URL, details);
      console.log(res);
    } catch (e) {
      console.error(e);
    }
  };
  //   const deleteEvent = (event) =>{

  //   }
  return (
    <>
      <div className="formDiv">
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
            >
              Delete
            </button>
          </div>
        </form>
      </div>
    </>
  );
};

export default RegForm;
