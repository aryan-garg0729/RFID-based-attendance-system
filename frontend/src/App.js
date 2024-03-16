import React from "react";
import "./App.css";
import RegForm from "./Components/RegForm/RegForm.js";
import Home from "./Components/Home/Home.js";
import NotFound from "./Components/NotFound/NotFound.js";
import FetchAll from "./Components/FetchAll/FetchAll.js";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import Header from "./Components/Header/Header.js";
function App() {
  return (
    <Router>
      <Header />
      <Routes>
        <Route exact path="/" Component={Home} />
        <Route exact path="/RegForm" Component={RegForm} />
        <Route exact path="/fetchAll" Component={FetchAll} />
        <Route path="*" Component={NotFound} />
      </Routes>
    </Router>
  );
}

export default App;
