import React from "react";

const GlobalFilter = ({ filter, setFilter }) => {
  return (
    <div style={{ textAlign: "center" }}>
      Search:{" "}
      <input
        className="form-input-style "
        value={filter || ""}
        onChange={(e) => setFilter(e.target.value)}
      />
    </div>
  );
};

export default GlobalFilter;
