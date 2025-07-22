import { initializeApp } from "https://www.gstatic.com/firebasejs/9.6.1/firebase-app.js";
import {
  get,
  getDatabase,
  limitToLast,
  query,
  ref,
  onValue,
} from "https://www.gstatic.com/firebasejs/9.6.1/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyBfcwUFSmItI4ciytYSEItcSJL_yma6Nmg",
  authDomain: "cosmopklora.firebaseapp.com",
  databaseURL:
    "https://cosmopklora-default-rtdb.europe-west1.firebasedatabase.app",
  projectId: "cosmopklora",
  storageBucket: "cosmopklora.appspot.com",
  messagingSenderId: "93822680518",
  appId: "1:93822680518:web:22a376eeb93148dd10c6c7",
  databaseURL:
    "https://cosmopklora-default-rtdb.europe-west1.firebasedatabase.app/",
};

export const app = initializeApp(firebaseConfig);
export const db = getDatabase(app);

const dbRef = ref(db);

const entryLimit = 80;

let tempChart = null;
let pressureChart = null;
let altitudeChart = null;

let data = {};
let lastN = 0;
const polylineData = [];
let polyline;

function cloneObject(object) {
  return JSON.parse(JSON.stringify(object));
}

const defaultData = {
  bmp: {
    altitude: 0,
    pressure: 0,
    temperature: 0,
  },
  gps: {
    latitude: 0,
    longitude: 0,
    time: {
      hours: 0,
      minutes: 0,
      seconds: 0,
    },
  },
  mpu: {
    accelerationAverage: {
      x: 0,
      y: 0,
      z: 0,
    },
    accelerationMax: {
      x: 0,
      y: 0,
      z: 0,
    },
    gyroscopeAverage: {
      x: 0,
      y: 0,
      z: 0,
    },
    gyroscopeMax: {
      x: 0,
      y: 0,
      z: 0,
    },
    rotationAverage: {
      w: 0,
      x: 0,
      y: 0,
      z: 0,
    },
  },
  n: 0,
  status: 0,
};

function pushToArrayAndTrim(array, data) {
  array.push(data);
  if (array.length > entryLimit) {
    array.splice(0, 1);
  }
}

function pushPolylineData(lat, lng) {
  pushToArrayAndTrim(polylineData, [lat, lng]);
  polyline.setLatLngs(polylineData);
}

function backfillRoot(incomingData, key) {
  if (!(key in incomingData)) {
    data[key] = defaultData[key];
  }
}

function backfillComponent(incomingData, component, key) {
  if (!(key in incomingData[component])) {
    data[component][key] = defaultData[component][key];
  }
}

function setData(incomingData) {
  try {
    data = incomingData;

    if (Object.keys(data).length === 0) {
      data = cloneObject(defaultData);
    }

    backfillRoot(incomingData, "n");
    backfillRoot(incomingData, "status");

    backfillRoot(incomingData, "bmp");
    backfillComponent(incomingData, "bmp", "altitude");
    backfillComponent(incomingData, "bmp", "pressure");
    backfillComponent(incomingData, "bmp", "temperature");

    backfillRoot(incomingData, "gps");
    backfillComponent(incomingData, "gps", "latitude");
    backfillComponent(incomingData, "gps", "longitude");
    backfillComponent(incomingData, "gps", "time");

    backfillRoot(incomingData, "mpu");
    backfillComponent(incomingData, "mpu", "accelerationAverage");
    backfillComponent(incomingData, "mpu", "accelerationMax");
    backfillComponent(incomingData, "mpu", "gyroscopeAverage");
    backfillComponent(incomingData, "mpu", "gyroscopeMax");
    backfillComponent(incomingData, "mpu", "rotationAverage");

    const time = data.gps.time;

    pushChartData(tempChart, data.bmp.temperature, time);
    pushChartData(pressureChart, data.bmp.pressure, time);
    pushChartData(altitudeChart, data.bmp.altitude, time);

    pushPolylineData(data.gps.latitude, data.gps.longitude);
  } catch (err) {
    data = cloneObject(defaultData);
    console.log("error", err);
  }
}

function pushChartData(chart, data, time) {
  pushToArrayAndTrim(
    chart.data.labels,
    formatDate(time.hours, time.minutes, time.seconds),
  );
  chart.data.datasets.forEach((dataset) => {
    pushToArrayAndTrim(dataset.data, data);
  });
  chart.update();
}

function formatDate(hours = 0, minutes = 0, seconds = 0) {
  return `${hours.toString().padStart(2, "0")}:${minutes.toString().padStart(2, "0")}:${seconds.toString().padStart(2, "0")}`;
}

function initChart(handle, name, color) {
  const ctx = document.querySelector(`#${handle}-chart`);
  return new Chart(ctx, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: name,
          data: [],
          fill: true,
          borderColor: `rgb(${color.r}, ${color.g}, ${color.b})`,
          tension: 0.1,
        },
      ],
    },
    options: {
      responsive: true,
    },
  });
}

function initCharts() {
  tempChart = initChart("temp", "Temperature", { r: 79, g: 192, b: 192 });
  pressureChart = initChart("pressure", "Pressure", { r: 192, g: 79, b: 192 });
  altitudeChart = initChart("altitude", "Altitude", { r: 192, g: 192, b: 79 });
}

async function retrieveHistoricalSnapshots(loraRef) {
  const historicalData = Object.values((await get(loraRef)).val())[0];
  const historicalSnapshots = Object.values(historicalData);
  return historicalSnapshots;
}

async function retrieveLatestSnapshot(loraRef) {
  const snapshots = await retrieveHistoricalSnapshots(loraRef);
  const latestSnapshot = snapshots[snapshots.length - 1];
  return latestSnapshot;
}

async function init() {
  initCharts();
  const loraRef = query(dbRef, limitToLast(1));
  let initialCoordinates = [50.0647, 19.945];
  {
    const latestSnapshot = await retrieveLatestSnapshot(loraRef);
    initialCoordinates = [
      latestSnapshot.gps.latitude ?? initialCoordinates[0],
      latestSnapshot.gps.longitude ?? initialCoordinates[1],
    ];
  }
  const map = L.map("map").setView(initialCoordinates, 13);

  L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 19,
    attribution:
      '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>',
  }).addTo(map);

  const marker = L.marker(initialCoordinates).addTo(map);
  polyline = new L.Polyline([], {
    color: "red",
    weight: 3,
    opacity: 0.5,
    smoothFactor: 1,
  }).addTo(map);

  {
    const historicalSnapshots = await retrieveHistoricalSnapshots(loraRef);
    const skipCount = 4; // snapshots are apparently taken 4 times a second
    for (let i = 0; i < historicalSnapshots.length; i += skipCount) {
      const snapshot = historicalSnapshots[i];
      setData(snapshot);
    }
  }

  function setMarkerLocation(lat, lng) {
    const newLoc = new L.LatLng(lat, lng);
    marker.setLatLng(newLoc);
  }

  onValue(loraRef, (snapshot) => {
    const latestData = Object.values(snapshot.val())[0];
    const snapshots = Object.values(latestData);
    const data = snapshots[snapshots.length - 1];
    if (data.n) {
      if (data.n == lastN) {
        return;
      }
      setData(data);
      map.flyTo([data.gps.latitude, data.gps.longitude], 14);
      setMarkerLocation(data.gps.latitude, data.gps.longitude);
      lastN = data.n;
    }
  });
}

init();
