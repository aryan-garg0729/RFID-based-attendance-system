import React, { useEffect } from "react";
import "./UserDashboard.css";
const UserDashboard = (props) => {
  // const [details, setDetails] = useState([]);
  useEffect(() => {
    document.body.style.overflowY = "hidden";
    return () => {
      document.body.style.overflowY = "scroll";
    };
  });
  return (
    <>
      <div className="dash-wrapper">
        <div className="dash-container">
        <div className="dash-header"><span>Student Details</span> <button className="close-btn">X</button></div>
          <div className="detail-section">
            <div className="dash-details">
              <div>
                <span>Name</span>
                <input type="text" />
              </div>
              <div>
                <span>Roll_no</span>
                <input type="text" />
              </div>
              <div>
                <span>RFID</span>
                <input type="text" />
              </div>
              <div>
                <span>CheckedIn Status</span>
                <input type="text" />
              </div>
            </div>
            <div className="dash-buttons">
              <button className="btn">Edit</button>
              <button className="btn">Delete</button>
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
          </div>
        </div>
      </div>
    </>
  );
};

export default UserDashboard;
