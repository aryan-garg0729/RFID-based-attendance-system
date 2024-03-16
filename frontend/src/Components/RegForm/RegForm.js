import React, { useState }from "react";
import "./RegForm.css";
const RegForm = () => {

    let [details, setDetails] = useState({tagId:"", name:"", rollNumber:"", gender:"", phone:""});

    const inputEvent=(event)=>{

        let value = event.target.value;
        let name = event.target.name;


        // 🔴🔴 METHOD ONE: USING OBJECT DESTRUCTURING
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
                {/* //🔴🔴⚠️ value attribute ke bina, form mei input data type hote hue show nahi hoga!! , but input to hoga! */}
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