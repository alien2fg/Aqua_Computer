import { ref, set, onValue, update } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js';
import { getAuth, onAuthStateChanged } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-auth.js';
import { database } from './firebase-config.js';

document.addEventListener('DOMContentLoaded', function () {
    const auth = getAuth();
    const dataRef = ref(database, '/aquariumData');

    // Function to update UI with data
    function updateUI(data) {
        document.getElementById('temperature').textContent = data.temperature || 'N/A';
        document.getElementById('ph').textContent = data.ph || 'N/A';
        document.getElementById('tds').textContent = data.tds || 'N/A';
        document.getElementById('sunrise').textContent = data.sunrise || 'N/A';
        document.getElementById('sunset').textContent = data.sunset || 'N/A';
        document.getElementById('feedingTimes').textContent = data.feedingTimes?.join(', ') || 'N/A';
        document.getElementById('feedAmount').textContent = data.feedAmount ? `${data.feedAmount} grams` : 'N/A';
    }

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

    // Initialize feeding times options based on default value
    updateFeedingTimesOptions(1); // Default to 1 feeding time

    onAuthStateChanged(auth, (user) => {
        if (user) {
            console.log(`Authenticated as: ${user.email}`);

            // Pobieranie danych i aktualizacja UI
            onValue(dataRef, (snapshot) => {
                const data = snapshot.val();
                updateUI(data || {});
            });

            // Ensure feedingCount element exists before adding event listener
            const feedingCountElement = document.getElementById('feedingCount');
            if (feedingCountElement) {
                feedingCountElement.addEventListener('change', (event) => {
                    const count = parseInt(event.target.value, 10);
                    updateFeedingTimesOptions(count);
                });
            }

            // Event listener for the form to simulate data update
            const dataForm = document.getElementById('dataForm');
            if (dataForm) {
                dataForm.addEventListener('submit', (event) => {
                    event.preventDefault();

                    const sunriseInput = document.getElementById('sunriseInput');
                    const sunsetInput = document.getElementById('sunsetInput');
                    const feedingCountElement = document.getElementById('feedingCount');
                    const feedAmountInput = document.getElementById('feedAmountInput');

                    const sunrise = sunriseInput?.value || '';
                    const sunset = sunsetInput?.value || '';
                    const feedingCount = parseInt(feedingCountElement?.value || 0, 10);
                    const feedAmount = parseFloat(feedAmountInput?.value || 0);

                    const feedingTimes = Array.from({ length: feedingCount }, (_, i) => {
                        const feedingTimeInput = document.getElementById(`feedingTime${i + 1}`);
                        return feedingTimeInput?.value || '';
                    });

                    const data = { sunrise, sunset, feedingTimes, feedAmount };
                    set(dataRef, data).then(() => console.log('Data updated successfully.'));
                });

            }

            // Feed Now button functionality
            const feedNowBtn = document.getElementById('feedNowBtn');
            if (feedNowBtn) {
                feedNowBtn.addEventListener('click', () => {
                    update(dataRef, { feedNow: 1 }).then(() => console.log('Feed Now triggered.'));
                });
            }
        } else {
            window.location.href = 'index.html'; // Przekierowanie do logowania
        }

        // Add event listener to the logout button
        const logoutBtn = document.getElementById('logoutBtn');
        if (logoutBtn) {
            logoutBtn.addEventListener('click', () => {
                signOut(auth)
                    .then(() => {
                        console.log('User signed out successfully');
                        window.location.href = 'index.html'; // Redirect to login page
                    })
                    .catch((error) => {
                        console.error('Error signing out:', error);
                    });
            });
        } else {
            console.error('Logout button not found.');
        }

    });
});
