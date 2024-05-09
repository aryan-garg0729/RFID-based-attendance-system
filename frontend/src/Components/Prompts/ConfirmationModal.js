import React, { useEffect } from "react";
import ReactDOM from "react-dom";
import "./ConfirmationModal.css";

// we need ReactDOM.createPortal to prevent any parent div to provide nice positioning
const ConfirmationModal = ({ onCancel, onConfirm }) => {
  useEffect(() => {
    document.body.style.overflowY = "hidden";
    return () => {
      document.body.style.overflowY = "scroll";
    };
  });

  return ReactDOM.createPortal(
    <>
      <div clasName="modal-wrapper" onClick={onCancel}>
        <div className="modal-container">
          <h5>Are you sure? </h5>
          <hr />
          <div className="modal-button-box">
            <button className="btn" onClick={onCancel} autoFocus>
              Cancel
            </button>
            <button className="btn deleteBtn" onClick={onConfirm}>
              Confirm
            </button>
          </div>
        </div>
      </div>
    </>,
    document.getElementById("modalRoot")
  );
};

export default ConfirmationModal;
