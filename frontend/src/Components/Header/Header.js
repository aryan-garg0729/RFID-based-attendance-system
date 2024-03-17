import React from "react";
import Container from "react-bootstrap/Container";
import Nav from "react-bootstrap/Nav";
import Navbar from "react-bootstrap/Navbar";
import { NavLink } from "react-router-dom";
import "./Header.css";

const Header = () => {
  return (
    <>
      <Navbar bg="primary" data-bs-theme="dark" className="navbar">
        <Container>
          <Nav className="me-auto">
            <Nav.Link as={NavLink} to="/" >
              Home
            </Nav.Link>
            <Nav.Link as={NavLink} to="/RegForm" >
              Register
            </Nav.Link>
            <Nav.Link as={NavLink} to="/fetchAll" >
              GetAll
            </Nav.Link>
          </Nav>
        </Container>
      </Navbar>
    </>
  );
};

export default Header;
