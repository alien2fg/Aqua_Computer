import firebase from "firebase/compat/app";
import "firebase/compat/database";
import config from './config.js'; // Import config

// Initialize Firebase
if (!firebase.apps.length) {
    firebase.initializeApp(config);
}

const db = firebase.database();

export default db;