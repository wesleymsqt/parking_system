// Function to fetch parking data from the server
async function fetchParkingData() {
    try {
        const response = await fetch('/api/data');
        const data = await response.json();
        updateParkingUI(data);
    } catch (error) {
        console.error('Error fetching parking data:', error);
    }
}

// Function to update the UI based on parking data
function updateParkingUI(data) {
    const spaces = [
        { id: 'space-1', status: data.parking_space_01 },
        { id: 'space-2', status: data.parking_space_02 },
        { id: 'space-3', status: data.parking_space_03 },
        { id: 'space-4', status: data.parking_space_04 },
    ];

    let occupiedCount = 0;

    spaces.forEach((space) => {
        const spaceElement = document.getElementById(space.id);
        const carImage = spaceElement.querySelector('.car-image');
        const timerElement = spaceElement.querySelector('.timer');

        if (space.status === 'occupied') {
            spaceElement.style.backgroundColor = 'red';
            carImage.style.display = 'block';
            timerElement.style.display = 'block';
            occupiedCount++;

            // Start or update the timer
            if (!spaceElement.entryTime) {
                spaceElement.entryTime = new Date(); // Record entry time
            }
            const elapsedTime = Math.floor((new Date() - spaceElement.entryTime) / 1000 / 60); // in minutes
            timerElement.textContent = `${elapsedTime} min`;
        } else {
            spaceElement.style.backgroundColor = 'green';
            carImage.style.display = 'none';
            timerElement.style.display = 'none';
            spaceElement.entryTime = null; // Reset entry time
        }
    });

    // Calculate and display occupancy rate
    const totalSpaces = spaces.length;
    const occupancyRate = ((occupiedCount / totalSpaces) * 100).toFixed(2);
    document.getElementById('rate').textContent = `${occupancyRate}%`;
}

// Fetch parking data every 5 seconds
setInterval(fetchParkingData, 3000);

// Initial fetch
fetchParkingData();