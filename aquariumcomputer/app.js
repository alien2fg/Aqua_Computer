// app.js

import { ref, set, onValue } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js';
import { database } from './firebase-config.js';

document.addEventListener('DOMContentLoaded', function() {
    // Reference to the data location
    const dataRef = ref(database, '/exampleData');

    // Function to add random data
    function addRandomData() {
        const randomData = {
            name: "John Doe",
            age: Math.floor(Math.random() * 100),
            email: "john.doe@example.com"
        };

        set(dataRef, randomData).then(() => {
            console.log('Data added successfully');
        }).catch((error) => {
            console.error('Error adding data:', error);
        });
    }

    // Event listener for the button
    document.getElementById('addData').addEventListener('click', addRandomData);

    // Fetch and display data
    onValue(dataRef, (snapshot) => {
        const data = snapshot.val();
        if (data) {
            document.getElementById('data').textContent = JSON.stringify(data, null, 2);
        } else {
            document.getElementById('data').textContent = 'No data available';
        }
    }, (error) => {
        document.getElementById('data').textContent = 'Error fetching data: ' + error.message;
    });
});
