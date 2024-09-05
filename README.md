
# Aquarium Monitoring System

## Project Overview

This project is a web-based monitoring system for an aquarium, built using HTML, CSS, and JavaScript, with Firebase Realtime Database for real-time data storage and updates. It displays key parameters such as temperature, pH level, TDS (Total Dissolved Solids), sunrise/sunset times, and feeding times of the aquarium.

The project allows users to:
- Monitor current aquarium conditions.
- Update aquarium data such as temperature, pH, TDS, sunrise, sunset, and feeding times.
- View data in real-time, automatically updated from the Firebase database.

## Features

- **Real-time Monitoring**: Data such as temperature, pH, TDS, and more is retrieved and updated in real-time from Firebase.
- **Firebase Integration**: Uses Firebase Realtime Database to store and retrieve aquarium data.
- **Responsive UI**: Displays information in a clean, visually appealing manner with flexbox-based layout and thematic backgrounds.
- **Customizable Background**: The page features an aquarium-themed background for a more immersive experience.

## Technologies Used

- **Frontend**: 
  - HTML5
  - CSS3 (with Flexbox for layout)
  - JavaScript (for interaction and Firebase integration)
  
- **Backend**:
  - **Firebase**: Firebase Realtime Database is used to store and update the aquarium data in real time.
  
## Project Structure

```bash
├── index.html         # Main HTML file with the structure of the page
├── app.js             # JavaScript file to handle Firebase operations and UI updates
├── firebase-config.js # Firebase configuration file (your API keys and database details)
├── config.js          # Custom configuration (if any)
├── README.md          # Project documentation
```

## How to Set Up the Project

### Prerequisites

1. A Firebase account and project set up with a Realtime Database.
2. Basic knowledge of HTML, JavaScript, and Firebase.
3. A modern browser with developer tools (e.g., Chrome, Firefox).

### Steps to Run the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/aquarium-monitoring.git
   cd aquarium-monitoring
   ```

2. **Set up Firebase:**
   - Create a Firebase project in the Firebase console.
   - Set up Firebase Realtime Database in your project.
   - Obtain your Firebase project configuration details (API key, project ID, etc.).

3. **Configure Firebase in the project:**
   - Replace the contents of `firebase-config.js` with your Firebase project's configuration.

   ```javascript
   // firebase-config.js
   import { initializeApp } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-app.js';
   import { getDatabase } from 'https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js';

   const firebaseConfig = {
       apiKey: "YOUR_API_KEY",
       authDomain: "YOUR_PROJECT_ID.firebaseapp.com",
       databaseURL: "https://YOUR_PROJECT_ID.firebaseio.com",
       projectId: "YOUR_PROJECT_ID",
       storageBucket: "YOUR_PROJECT_ID.appspot.com",
       messagingSenderId: "YOUR_MESSAGING_SENDER_ID",
       appId: "YOUR_APP_ID"
   };

   const app = initializeApp(firebaseConfig);
   export const database = getDatabase(app);
   ```

4. **Structure of the Firebase database**:
   Make sure your Firebase Realtime Database follows this structure for the aquarium data:

   ```json
   {
     "aquariumData": {
       "temperature": 25.5,
       "ph": 7.2,
       "tds": 350,
       "sunrise": "06:00 AM",
       "sunset": "08:00 PM",
       "feedingTimes": ["08:00 AM", "07:00 PM"]
     }
   }
   ```

5. **Open the project:**
   Open `index.html` in a web browser to see the monitoring system in action.

### Customization

- **Background Image**: You can change the aquarium-themed background by modifying the `background` property in the `body` selector in `index.html`. Simply provide a new URL for the `background-image`:

   ```css
   body {
       background: url('YOUR_NEW_IMAGE_URL') no-repeat center center fixed;
       background-size: cover;
   }
   ```

## Firebase Database Structure

The Firebase Realtime Database should have a structure like this:

```json
{
  "aquariumData": {
    "temperature": 25.0,
    "ph": 7.5,
    "tds": 400,
    "sunrise": "06:00 AM",
    "sunset": "08:00 PM",
    "feedingTimes": ["08:00 AM", "06:00 PM"]
  }
}
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Background image from [Unsplash](https://unsplash.com).
- Firebase for providing free hosting and real-time database services.
