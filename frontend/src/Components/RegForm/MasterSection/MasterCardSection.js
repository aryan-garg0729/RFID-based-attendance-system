import React, { useState, useEffect } from "react";
import axios from "axios";
import MasterCard from "./MasterCard";
import { toast } from "react-toastify";
import errorToast from "../../Prompts/ErrorToast";

const MASTER_URL = "http://localhost:4000/admin/master/names/";

const MasterCardSection = () => {
  const [names, setNames] = useState([]);
  useEffect(() => {
    fetchNames();
  }, []);

  const fetchNames = async () => {
    try {
      const response = await axios.get(MASTER_URL); // Use the GET /names route
      // console.log(response.data);
      setNames(response.data);
    } catch (error) {
      console.error("Error fetching names:", error);
      errorToast(error);
    }
  };

  const updateName = async (index, newName) => {
    try {
      const res = await axios.put(MASTER_URL + index, { name: newName });
      // await fetchNames(); // Refresh the list
      toast(`${res.status} | ${res.data.message}`);
    } catch (error) {
      console.error("Error updating name:", error);
      errorToast(error);
    }
  };

  const deleteName = async (index) => {
    try {
      const res = await axios.delete(MASTER_URL + index);
      // await fetchNames(); // Refresh the list
      toast(`${res.status} | ${res.data.message}`);
    } catch (error) {
      console.error("Error deleting name:", error);
      errorToast(error);
    }
  };

  return (
    <div className="master-list">
      {names.length ? <h3>Masters</h3> : <></>}
      {names.map((val, index) => {
        return (
          <MasterCard
            key={index}
            id={index}
            rfid={val}
            updateName={updateName}
            deleteName={deleteName}
          />
        );
      })}
    </div>
  );
};

export default MasterCardSection;
