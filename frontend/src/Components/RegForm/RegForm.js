import React, { useState, useEffect }from "react";
import axios from 'axios';
import "./RegForm.css";





const RegForm = () => {
    const [data, setData] = useState({ hits: [] });
    let [details, setDetails] = useState({tagId:"", name:"", rollNumber:"", gender:"", phone:""});

    useEffect(async () => {
      const result = await axios(
        'https://hn.algolia.com/api/v1/search?query=redux',
      );
  
      setData(result.data);
    });



    const inputEvent=(event)=>{

        let value = event.target.value;
        let name = event.target.name;


        // set details onChange event
        setDetails( (previousValue) => {
           return {...previousValue,
            [name] : value}
        }); 

    }

    //form's default action webpage is refreshing the webpage hence we lost the user value
    const actionSubmit = (event) => {
        event.preventDefault();
        console.log(details);

    }


    return (
        <>
            <div className='formDiv'>
                <form onSubmit={actionSubmit}>
                    <input type="text" placeholder="RFID Tag ID" name="tagId" value={details.tagId} onChange={inputEvent}/>
                    <input type="text" placeholder="Name" name="name" value={details.name} onChange={inputEvent} />
                    <input type="text" placeholder="RollNumber" name="rollNumber" value={details.rollNumber} onChange={inputEvent} />
                    <input type="text" placeholder="Gender" name="gender" value={details.gender} onChange={inputEvent} />
                    <input type="text" placeholder="Validity (default one month from now)" name="gender" value={details.gender} onChange={inputEvent} />
                    <input type="text" placeholder="Phone" name="phone" value={details.phone} onChange={inputEvent}/>
                    <div className="buttonBox">
                        <button type='submit' className="btn submitButton" id="submitButton">Submit</button>
                        <button type='submit' className="btn deleteButton" id="deleteButton">Delete</button>
                    </div>
                </form>
            </div>

        </>
    );
}



export default RegForm;