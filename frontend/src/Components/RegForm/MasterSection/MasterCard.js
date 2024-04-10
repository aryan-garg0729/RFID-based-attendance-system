import React, { useState } from "react";
import ConfirmationModal from "../../Prompts/ConfirmationModal";

const MasterCard = (props) => {
  const [isEdit, setIsEdit] = useState(false);
  const [value, setValue] = useState(props.rfid);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [showUpdateModal, setShowUpdateModal] = useState(false);

  // close confirmation Modal
  const closeModal = () => {
    setShowDeleteModal(false);
    setShowUpdateModal(false);
  };

  const handleDoubleClick = () => {
    setIsEdit(true);
  };

  const handleChange = (event) => {
    setValue(event.target.value);
  };

  const handleBlur = () => {
    setTimeout(() => {
      if (isEdit) {
        setIsEdit(false);
      }
    }, 300); // Adjust the delay as needed
  };

  const handleUpdateName = () => {
    props.updateName(props.id, value);
  };

  const handleDeleteName = () => {
    props.deleteName(props.id);
  };
  return (
    <>
      {/* Delete confirmation Modal */}
      {showDeleteModal && (
        <ConfirmationModal onCancel={closeModal} onConfirm={handleDeleteName} />
      )}
      {/* Update confirmation Modal */}
      {showUpdateModal && (
        <ConfirmationModal onCancel={closeModal} onConfirm={handleUpdateName} />
      )}
      <div
        className="master"
        onDoubleClick={handleDoubleClick}
        onBlur={handleBlur}
      >
        {isEdit ? (
          <>
            <input
              type="text"
              value={value}
              onChange={handleChange}
              autoFocus
            />
            <div className="master-btn-box">
              <button
                className="btn saveBtn"
                onClick={() => setShowUpdateModal(true)}
              >
                Save
              </button>
              <button
                className="btn deleteBtn"
                onClick={() => setShowDeleteModal(true)}
              >
                Delete
              </button>
            </div>
          </>
        ) : (
          <span>{value}</span>
        )}
      </div>
    </>
  );
};

export default MasterCard;
