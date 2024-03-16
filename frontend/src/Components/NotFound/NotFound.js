import React from 'react';
import Button from 'react-bootstrap/Button';
import { Link } from 'react-router-dom';


const NotFound = () => {
  return (
    <div>
        <h3>Page Not Found</h3>
        <Link to="/"> <Button variant="primary">Primary</Button></Link>
    </div>
  )
}

export default NotFound