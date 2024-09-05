// app.js

import { ref, set, onValue } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js';
import { database } from './firebase-config.js';

document.addEventListener('DOMContentLoaded', function() {
    const dataRef = ref(database, '/aquariumData');
    
    // Function to update UI with data
    function updateUI(data) {
        document.getElementById('temperature').textContent = data.temperature || 'N/A';
        document.getElementById('ph').textContent = data.ph || 'N/A';
        document.getElementById('tds').textContent = data.tds || 'N/A';
        document.getElementById('sunrise').textContent = data.sunrise || 'N/A';
        document.getElementById('sunset').textContent = data.sunset || 'N/A';
        document.getElementById('feedingTimes').textContent = data.feedingTimes ? data.feedingTimes.join(', ') : 'N/A';
    }

    // Fetch and display data
    onValue(dataRef, (snapshot) => {
        const data = snapshot.val();
        if (data) {
            updateUI(data);
        } else {
            updateUI({});
        }
    }, (error) => {
        document.getElementById('data').textContent = 'Error fetching data: ' + error.message;
    });

    // Event listener for the button to simulate data update
    const updateButton = document.getElementById('updateData');
    
    if (updateButton) {
        updateButton.addEventListener('click', () => {
            const data = {
                temperature: (Math.random() * 10 + 20).toFixed(1), // Random temperature between 20°C and 30°C
                ph: (Math.random() * 2 + 6).toFixed(1), // Random pH between 6 and 8
                tds: Math.floor(Math.random() * 1000 + 200), // Random TDS between 200 and 1200 ppm
                sunrise: '06:00 AM', // Example value
                sunset: '08:00 PM', // Example value
                feedingTimes: ['08:00 AM', '07:00 PM'] // Example feeding times
            };

            // Update Firebase with new data
            set(dataRef, data).then(() => {
                console.log('Data updated successfully');
            }).catch((error) => {
                console.error('Error updating data:', error);
            });
        });
    }
});
