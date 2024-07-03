let data = {};

let tempChart = null;
let pressureChart = null;
let altitudeChart = null;

async function fetchData() {
  return JSON.parse(await (await fetch("/get")).json());
}

function cloneObject(object) {
  return JSON.parse(JSON.stringify(object));
}

const defaultData = {
  time: 0.0,
  bme: {
    temperature: 0.0,
    humidity: 0.0,
    pressure: 0.0,
  },
  gps: {
    latitude: 55.7506,
    longitude: 37.6175,
    altitude: 0.0,
  },
  mpu: {
    acceleration: { x: 0.0, y: 0.0, z: 0.0 },
    rotation: { x: 0.0, y: 0.0, z: 0.0 },
    angularVelocity: { x: 0.0, y: 0.0, z: 0.0 },
  },
};

async function setData() {
  try {
    data = await fetchData();
    if (Object.keys(data).length === 0) {
      data = cloneObject(defaultData);
    }

    pushChartData(tempChart, data.bme.temperature);
    pushChartData(pressureChart, data.bme.pressure);
    pushChartData(altitudeChart, data.gps.altitude);
  } catch {
    data = cloneObject(defaultData);
  }
}

function pushChartData(chart, data) {
  chart.data.labels.push(formatDate());
  chart.data.datasets.forEach((dataset) => {
    dataset.data.push(data);
    if (dataset.data.length > 50) {
      dataset.data.splice(0, 1);
    }
  });
  chart.update();
}

function formatDate() {
  const date = new Date();
  return `${date.getHours().toString().padStart(2, "0")}:${date.getMinutes().toString().padStart(2, "0")}:${date.getSeconds().toString().padStart(2, "0")}`;
}

function initChart(handle, name, color) {
  const ctx = document.querySelector(`#${handle}-chart`);
  return new Chart(ctx, {
    type: "line",
    data: {
      labels: [formatDate()],
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

async function init() {
  initCharts();
  await setData();

  const map = L.map("map").setView([data.gps.latitude, data.gps.longitude], 13);

  L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 19,
    attribution:
      '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>',
  }).addTo(map);

  const marker = L.marker([data.gps.latitude, data.gps.longitude]).addTo(map);

  function setMarkerLocation(lat, lng) {
    const newLoc = new L.LatLng(lat, lng);
    marker.setLatLng(newLoc);
  }

  setInterval(async () => {
    await setData();
    map.flyTo([data.gps.latitude, data.gps.longitude], 14);
    setMarkerLocation(data.gps.latitude, data.gps.longitude);
  }, 1500);
}

init();
