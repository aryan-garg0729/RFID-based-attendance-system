# RFID-Based Robust Attendance System

---

## Overview

This project is an RFID-based attendance system designed to streamline attendance tracking while ensuring robustness and reliability. The system is capable of functioning even in scenarios where Wi-Fi connectivity is lost or the backend server fails, thanks to its offline capabilities and built-in battery backup. This reduces manual effort, enhances accuracy, and simplifies attendance management processes.

Explore a live demo of the project:
https://drive.google.com/drive/folders/1W0VWSmqyWlxAUKie9-PCyebDMAZzshVx

## Features

- **RFID Technology**: Utilizes RFID (Radio Frequency Identification) for quick and accurate identification of individuals.
- **Offline Functionality**: Capable of storing attendance data locally on the device in case of network unavailability.
- **Battery Backup**: Includes a battery backup system to ensure uninterrupted operation, even during power outages.
- **NodeMCU Integration**: Integrates NodeMCU (ESP8266) for Wi-Fi connectivity and microcontroller functionality.
- **MERN Stack Backend**: Utilizes the MERN (MongoDB, Express.js, React.js, Node.js) stack for the backend infrastructure, ensuring scalability and flexibility.
- **Arduino IDE**: Developed using the Arduino Integrated Development Environment (IDE) with C++ programming for microcontroller functionality.

## Components

- NodeMCU (ESP8266): Handles Wi-Fi connectivity and interfaces with the RFID reader.
- RFID Reader: Reads RFID tags/cards to identify individuals.
- Battery Backup System: Ensures continuous operation during power disruptions.
- Backend Server: Manages attendance data storage and retrieval using the MERN stack.
- Frontend Interface: Provides a user-friendly interface for administrators to monitor attendance records.

## Setup Instructions

1. **Hardware Setup**:
   - Connect NodeMCU to the RFID reader and ensure proper wiring.
     ![image](https://github.com/aryan-garg0729/RFID-based-attendance-system/assets/155893692/e76a1364-e20b-41e5-883b-ee844f7f3524)

   - Integrate the battery backup system to ensure uninterrupted operation.

2. **Software Setup**:
   - Install the necessary libraries and dependencies in the Arduino IDE.
   - Set up the MERN stack backend server and ensure connectivity with the NodeMCU.
     - To install dependencies in the MERN stack, run the following command:
       ```
       npm i
       ```
     - To run the frontend, use the following command:
       ```
       npm start
       ```
     - To run the backend, use the following command:
       ```
       npx nodemon
       ```

3. **Testing and Deployment**:
   - Test the system thoroughly under various conditions to ensure reliability.
   - Deploy the system in the desired environment, ensuring proper network connectivity and power supply.

## Usage

1. **Enrollment**:
   - Add individuals to the system by registering their RFID tags/cards along with their information.

2. **Attendance Tracking**:
   - Individuals can scan their RFID tags/cards upon arrival to mark their attendance.
   - Attendance data is stored locally and synced with the backend server when connectivity is available.

3. **Monitoring and Reporting**:
   - Administrators can monitor attendance records through the frontend interface.
   - Generate reports and analyze attendance trends for further insights.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

- Special thanks to contributors and open-source libraries used in this project.
- Inspiration drawn from the need for efficient attendance tracking solutions in various industries.

---

