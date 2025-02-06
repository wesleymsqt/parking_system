const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');

const app = express();
const port = 3000;

// Middleware to parse JSON bodies
app.use(bodyParser.json());

// Serve static files (HTML, CSS, JS)
app.use(express.static(path.join(__dirname, 'public')));

// Store the latest parking data
let latestParkingData = {
    parking_space_01: 'available',
    parking_space_02: 'available',
    parking_space_03: 'available',
    parking_space_04: 'available',
};

// Endpoint to handle POST requests from ESP32 devices
app.post('/api/data', (req, res) => {
    const data = req.body;

    // Update the latest parking data
    latestParkingData = {
        parking_space_01: data.parking_space_01,
        parking_space_02: data.parking_space_02,
        parking_space_03: data.parking_space_03,
        parking_space_04: data.parking_space_04,
    };

    // Log the received data
    console.log(`Received data from device: ${data.device_id}`);
    console.log('Parking Spaces Status:', latestParkingData);

    // Send a response back to the ESP32
    res.status(200).json({ status: 'success', message: 'Data received' });
});

// Endpoint to handle GET requests from the frontend
app.get('/api/data', (req, res) => {
    // Send the latest parking data to the frontend
    res.status(200).json(latestParkingData);
});

// Start the server
app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});