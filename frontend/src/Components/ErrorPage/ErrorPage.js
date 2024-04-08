import React from "react";
import { NavLink } from "react-router-dom";
import "./ErrorPage.css";

const ErrorPage = () => {
  return (
    <div className="error-container grid-center">
      <div className="error-message">
        <p className="error404"> Error 404 </p>
        <h2 className="notfound"> Page Not Found</h2>
        <p className="error-link">
          <NavLink className="error-homeLink" to="/">
            Go Home
          </NavLink>
        </p>
      </div>
    </div>
  );
};

export default ErrorPage;
