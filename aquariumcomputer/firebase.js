import firebase from "firebase/compat/app";
import "firebase/database";

const firebaseConfig = {
    apiKey: "AIzaSyCWCgA5Cw8jKgz6z_F9xNGQiU372IOFTo4",
    authDomain: "aqua-computer.firebaseapp.com",
    databaseURL: "https://aqua-computer-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "aqua-computer",
    storageBucket: "aqua-computer.appspot.com",
    messagingSenderId: "269598410424",
    appId: "1:269598410424:web:529e7d98dbc9350777d64d"
  };

if (!firebase.app.length) {
    firebase.initializeApp(firebaseConfig);
}

const db = firebase.database();

export default db;
