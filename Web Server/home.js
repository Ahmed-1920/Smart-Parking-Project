const firebaseConfig = {
    apiKey: "AIzaSyCKddWGVzR_pUZXHJzixfdyXWFtifmV3oM",
    authDomain: "iot-project-eu.firebaseapp.com",
    databaseURL: "https://iot-project-eu-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "iot-project-eu",
    storageBucket: "iot-project-eu.appspot.com",
    messagingSenderId: "622931504434",
    appId: "1:622931504434:web:9520b36b780a3175d67b2b"
  };

  firebase.initializeApp(firebaseConfig);
  var database = firebase.database();
  var auth = firebase.auth()
  
  let openEntranceStatus = false;
  let openExitStatus = false;
  let fireAlarmStatus = false;
  let carCount = 0;
  
  function toggleDoor(door) {
      if (door === 'door1') {
          openEntranceStatus = !openEntranceStatus;
          updateDoorStatus('openEntrance', openEntranceStatus);
      } else if (door === 'door2') {
          openExitStatus = !openExitStatus;
          updateDoorStatus('openExit', openExitStatus);
      }
  }
  
  function updateDoorStatus(door, status) {
      database.ref('INFO/' + door).set(status ? true : false);
  }
  
  function toggleFireAlarm() {
      fireAlarmStatus = !fireAlarmStatus;
      updateFireAlarmStatus(fireAlarmStatus);
  }
  
  function updateFireAlarmStatus(status) {
      document.getElementById('fire-alarm-status').innerText = status ? 'Active' : 'Inactive';
      database.ref('INFO/fireAlarm').set(status ? true : false);
  }
  
  database.ref('INFO/openEntrance').on('value', function(snapshot) {
      openEntranceStatus = snapshot.val() === true;
      document.getElementById('door1-status').innerText = openEntranceStatus ? 'Open' : 'Closed';
  });
  
  database.ref('INFO/openExit').on('value', function(snapshot) {
      openExitStatus = snapshot.val() === true;
      document.getElementById('door2-status').innerText = openExitStatus ? 'Open' : 'Closed';
  });
  
  database.ref('INFO/fireAlarm').on('value', function(snapshot) {
      fireAlarmStatus = snapshot.val() === true;
      document.getElementById('fire-alarm-status').innerText = fireAlarmStatus ? 'Active' : 'Inactive';
  });
  
  database.ref('INFO/carCount').on('value', function(snapshot) {
      carCount = snapshot.val();
      document.getElementById('car-count').innerText = carCount;
  });

  function logout() {
    auth.signOut().then(() => {
        // Sign-out successful
        console.log('User signed out.');
        window.location.href = 'index.html'; // Redirect to login page or home page
    }).catch((error) => {
        // An error happened
        console.error('Sign-out error:', error);
    });
}