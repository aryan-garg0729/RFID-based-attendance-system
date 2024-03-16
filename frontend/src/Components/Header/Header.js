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
            <Nav.Link>
              <NavLink to="/" className="navlink">
                Home
              </NavLink>
            </Nav.Link>
            <Nav.Link>
              <NavLink to="/RegForm" className="navlink">
                Register
              </NavLink>
            </Nav.Link>
            <Nav.Link>
              <NavLink to="/fetchAll" className="navlink">
                GetAll
              </NavLink>
            </Nav.Link>
          </Nav>
        </Container>
      </Navbar>
    </>
  );
};

export default Header;
