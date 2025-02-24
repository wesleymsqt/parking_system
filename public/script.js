// Buscar dados de estacionamento do servidor
async function fetchParkingData() {
    try {
        const response = await fetch('/api/data');
        const data = await response.json();
        updateParkingUI(data);
    } catch (error) {
        console.error('Error fetching parking data:', error);
    }
}

// Atualizar a IU com base nos dados de estacionamento
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

            // Iniciar ou atualizar o temporizador
            if (!spaceElement.entryTime) {
                spaceElement.entryTime = new Date();
            }
            timerElement.textContent = `${elapsedTime} min`;
        } else {
            spaceElement.style.backgroundColor = 'green';
            carImage.style.display = 'none';
            timerElement.style.display = 'none';
            spaceElement.entryTime = null; // Redefinir tempo de entrada
        }
    });

    // Calcular e exibir a taxa de ocupação
    const totalSpaces = spaces.length;
    const occupancyRate = ((occupiedCount / totalSpaces) * 100).toFixed(2);
    document.getElementById('rate').textContent = `${occupancyRate}%`;
}

// Obter dados de estacionamento a cada 5 segundos
setInterval(fetchParkingData, 3000);

fetchParkingData();