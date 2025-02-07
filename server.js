const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');

const app = express();
const port = 3000;

// Middleware to parse JSON bodies
app.use(bodyParser.json());

// Serve static files (HTML, CSS, JS)
app.use(express.static(path.join(__dirname, 'public')));

// Store the latest parking data and entry times
let latestParkingData = {
    parking_space_01: { status: 'available', entryTime: null },
    parking_space_02: { status: 'available', entryTime: null },
    parking_space_03: { status: 'available', entryTime: null },
    parking_space_04: { status: 'available', entryTime: null },
};

// Endpoint to handle POST requests from ESP32 devices
app.post('/api/data', (req, res) => {
    const data = req.body;

    // Update the latest parking data
    Object.keys(latestParkingData).forEach((space) => {
        const newStatus = data[space];
        const currentSpace = latestParkingData[space];

        if (newStatus === 'occupied' && currentSpace.status === 'available') {
            // Car just occupied the space: record the entry time
            currentSpace.entryTime = new Date();
        } else if (newStatus === 'available' && currentSpace.status === 'occupied') {
            // Car just left the space: calculate the elapsed time
            const exitTime = new Date();
            const elapsedTime = Math.floor((exitTime - currentSpace.entryTime) / 1000 / 60); // in minutes
            console.log(`Car left ${space}. Time spent: ${elapsedTime} minutes`);
        }

        // Update the status
        currentSpace.status = newStatus;
    });

    // Log the received data
    console.log(`Received data from device: ${data.device_id}`);
    console.log('Parking Spaces Status:', latestParkingData);

    // Send a response back to the ESP32
    res.status(200).json({ status: 'success', message: 'Data received' });
});

// Endpoint to handle GET requests from the frontend
app.get('/api/data', (req, res) => {
    // Send the latest parking data to the frontend
    const responseData = {};
    Object.keys(latestParkingData).forEach((space) => {
        responseData[space] = latestParkingData[space].status;
    });
    res.status(200).json(responseData);
});

// Start the server
app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});