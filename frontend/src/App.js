import React from "react";
import "./App.css";
import RegForm from "./Components/RegForm/RegForm.js";
import Home from "./Components/Home/Home.js";
import ErrorPage from "./Components/ErrorPage/ErrorPage";
import FetchAll from "./Components/FetchAll/FetchAll.js";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import Header from "./Components/Header/Header.js";
import UserDashboard from "./Components/UserDashboard/UserDashboard";

function App() {
  return (
    <>
      <Router>
        <Header />
        <Routes>
          {/* <Route exact path="/" Component={Home} /> */}
          <Route exact path="/" Component={RegForm} />
          <Route exact path="/fetchAll" Component={FetchAll} />
          <Route exact path="/userDashboard" Component={UserDashboard} />
          <Route path="*" Component={ErrorPage} />
        </Routes>
      </Router>
    </>
  );
}

export default App;
