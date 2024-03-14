import React, { useState }from "react";

const RegistrationForm = () => {

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
            <div>
                <h1>Hello {details.name}</h1>
            </div>
            <div className='formDiv'>
                {/* //🔴🔴⚠️ value attribute ke bina, form mei input data type hote hue show nahi hoga!! , but input to hoga! */}
                <form onSubmit={actionSubmit}>
                    <input type="text" placeholder="RFID Tag ID" name="tagId" value={details.tagId} onChange={inputEvent}/>
                    <input type="text" placeholder="Enter Name" name="name" value={details.name} onChange={inputEvent} />
                    <input type="text" placeholder="Enter rollNumber" name="rollNumber" value={details.rollNumber} onChange={inputEvent} />
                    <input type="text" placeholder="Enter Gender" name="gender" value={details.gender} onChange={inputEvent} />
                    <input type="text" placeholder="Enter phone" name="phone" value={details.phone} onChange={inputEvent}/>
                    <div className="buttonBox">
                        <button type='submit' className="btn createButton" id="createButton">Create</button>
                        <button type='submit' className="btn updateButton" id="updateButton">Update</button>
                        <button type='submit' className="btn deleteButton" id="deleteButton">Delete</button>
                    </div>
                </form>
            </div>

        </>
    );
}



export default RegistrationForm;