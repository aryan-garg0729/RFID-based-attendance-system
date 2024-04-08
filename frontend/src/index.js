import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App.js";
import { ToastContainer } from "react-toastify";
import "react-toastify/dist/ReactToastify.css";

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(
  <React.Fragment>
    <App />
    <ToastContainer
      position="top-center"
      autoClose={3001}
      limit={1}
      hideProgressBar={false}
      newestOnTop
      closeOnClick
      rtl={false}
      pauseOnFocusLoss
      draggable={false}
      pauseOnHover
      theme="light"
    />
  </React.Fragment>
);
