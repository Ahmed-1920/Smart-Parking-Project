# Smart Parking System

## Project Report

### Introduction
The Smart Parking System is designed to streamline garage operations by integrating modern web technologies and embedded systems. The project aims to provide a user-friendly platform for managing parking spaces, ensuring efficient and secure entry and exit of vehicles, and maintaining real-time monitoring of the garage status. By leveraging Firebase Authentication and Firebase Realtime Database, the system offers robust user management and data synchronization.

### Components Used
- **ESP32 Dev Kit V1:** The main microcontroller for controlling sensors and actuators.
- **IR Sensors:** For additional vehicle detection and counting.
- **SG90 DC Servo Motor:** For controlling the gate mechanism.
- **LCD Display (16x2):** For displaying system status and vehicle count.
- **4x4 Membrane Keypad:** For user input to control the system manually.
- **Buzzer:** For fire alarm alerts.
- **LEDs:** For visual indicators of system status.
- **Power Supply:** To power the ESP32 and other components.
- **Breadboard and Jumper Wires:** For prototyping and connections.

### Project Goal
The primary goal of the Smart Parking System project is to develop an automated parking management system that:
1. **Enhances Security:** Ensures only authenticated users can access the parking facility.
2. **Improves Efficiency:** Automates vehicle entry and exit processes to reduce manual intervention.
3. **Provides Real-Time Monitoring:** Displays the current status of the garage, including the number of available parking spaces and the state of the entrance and exit gates.
4. **Ensures Safety:** Includes a fire alarm system to open all gates and alert users in case of emergencies.

### Main Loop Flow and Functionality
The core functionality of the system is organized around a loop that includes the following processes:

#### Firebase Reconnection Check
The loop initiates by verifying if the Firebase connection is active. In the event of a disconnection, the system attempts to reconnect.

#### Fetching Data from Firebase
Every 2 seconds, the system retrieves data from Firebase to monitor the status of `openExit`, `openEntrance`, and `fireAlarm`.

#### State Machine Handling
A state machine is employed to manage various states of the parking system:
- **NO_REQ / FULL:**
  - If a car is detected and the parking is full, display "Parking is full".
  - If a car is detected and the parking is not full, transition to `WAITING_FOR_AUTHEN` state.
- **WAITING_FOR_AUTHEN:**
  - Wait for authentication (up to 5 seconds). If time exceeds, transition to `AUTHEN_FAILED` state.
- **AUTHEN_FAILED:**
  - Wait until the car leaves, then transition back to `NO_REQ` state.
- **CAR_ENTERING:**
  - If a car passes the gate sensor, increment the car count, close the gate, and transition to `NO_REQ`.
  - If no car is detected after a set time, close the gate and transition back to `NO_REQ`.
- **FIREALARM:**
  - Open all gates and activate the buzzer. Stay in this state until the fire alarm is deactivated from the server.

#### Keypad Input Handling
The system continuously checks for keypad inputs to manually open or close the entrance and exit gates based on specific key presses and holding duration.

### Main Connections
![Connection Diagram](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture2.png)
![Connection Diagram](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture1.jpg)

### Web Server

#### 1. User Authentication
##### Sign-In and Sign-Up Page
- **Sign-Up Page:** New users can create an account using their username, email, and password.
- **Login Page:** Registered users can log in using their credentials.
- The authentication process is handled by Firebase Authentication, ensuring secure and reliable user management.

#### 2. Home Page
The home page serves as the control center for the garage, providing the following functionalities:

##### Buttons for Garage Door Control
- **Entrance Button:** Opens and closes the entrance garage door.
- **Exit Button:** Opens and closes the exit garage door.

##### Fire Alarm Control
- **Fire Alarm Button:** Activates or deactivates the fire alarm system.

#### 3. Car Count Display
- **Car Counter:** Displays the current number of cars in the garage. This count is updated in real-time as cars enter and exit.

### Firebase Realtime Database
![Firebase Realtime Database](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture3.jpg)

### Firebase Authentication
![Firebase Authentication](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture5.png)

### User Interface
The user interface is designed to be user-friendly and intuitive:
- **Sign-In and Login Pages:** Simple forms for entering email and password.
- **Home Page:** Clearly labeled buttons for controlling the garage doors and fire alarm, and a displayed car counter.
![Sign in](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture6.png)
![Sign up](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture7.png)
![Home Screen](https://github.com/Ahmed-1920/Smart-Parking-Project/blob/main/images/Picture8.png)


Video Link (explained in Arabic)
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/rtqSXsmh-Kc/0.jpg)](https://www.youtube.com/watch?v=rtqSXsmh-Kc)
