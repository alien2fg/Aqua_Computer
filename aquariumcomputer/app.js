import { ref, set, onValue, update } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js';
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

    // Function to update feeding time options based on selected number
    function updateFeedingTimesOptions(count) {
        const container = document.getElementById('feedingTimesContainer');
        if (!container) return; // Ensure container exists

        container.innerHTML = '<label for="feedingTimesInput">Feeding Times:</label>';
        for (let i = 0; i < count; i++) {
            const input = document.createElement('input');
            input.type = 'time';
            input.id = `feedingTime${i + 1}`;
            input.required = true;
            input.placeholder = `Feeding Time ${i + 1}`;
            container.appendChild(input);
            container.appendChild(document.createElement('br'));
        }
    }

    // Ensure feedingCount element exists before adding event listener
    const feedingCountElement = document.getElementById('feedingCount');
    if (feedingCountElement) {
        feedingCountElement.addEventListener('change', (event) => {
            const count = parseInt(event.target.value, 10);
            updateFeedingTimesOptions(count);
        });
    } else {
        console.error('Element with ID "feedingCount" not found.');
    }

    // Event listener for the form to simulate data update
    const dataForm = document.getElementById('dataForm');
    if (dataForm) {
        dataForm.addEventListener('submit', (event) => {
            event.preventDefault();

            const sunriseInput = document.getElementById('sunriseInput');
            const sunsetInput = document.getElementById('sunsetInput');
            const feedingCountElement = document.getElementById('feedingCount');

            if (!sunriseInput || !sunsetInput || !feedingCountElement) {
                console.error('One or more form elements are missing.');
                return;
            }

            const sunrise = sunriseInput.value;
            const sunset = sunsetInput.value;
            const feedingCount = parseInt(feedingCountElement.value, 10);
            const feedingTimes = [];

            for (let i = 0; i < feedingCount; i++) {
                const feedingTimeInput = document.getElementById(`feedingTime${i + 1}`);
                if (feedingTimeInput) {
                    const feedingTime = feedingTimeInput.value;
                    feedingTimes.push(feedingTime);
                }
            }

            const data = {
                temperature: document.getElementById('temperature').textContent,  // Temperature will be static now
                ph: document.getElementById('ph').textContent, // pH is read-only
                sunrise: sunrise,
                sunset: sunset,
                feedingTimes: feedingTimes,
                tds: document.getElementById('tds').textContent, // TDS will also be static now
            };

            // Update Firebase with new data
            set(dataRef, data).then(() => {
                console.log('Data updated successfully');
            }).catch((error) => {
                console.error('Error updating data:', error);
            });
        });
    } else {
        console.error('Form element with ID "dataForm" not found.');
    }

    // Initialize feeding times options based on default value
    updateFeedingTimesOptions(1); // Default to 1 feeding time

    // Feed Now button functionality
    const feedNowBtn = document.getElementById('feedNowBtn');
    if (feedNowBtn) {
        feedNowBtn.addEventListener('click', () => {
            // Set feedNow to 1 inside aquariumData object
            update(dataRef, { feedNow: 1 }).then(() => {
                console.log('Feed Now triggered');
            }).catch((error) => {
                console.error('Error triggering Feed Now:', error);
            });
        });
    } else {
        console.error('Feed Now button not found.');
    }
});
