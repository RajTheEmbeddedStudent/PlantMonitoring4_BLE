// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

let filterdata;

// Define thresholds for each sensor
let thresholds = {
  temperatureMin: 0,   // Minimum Temperature (°C)
  temperatureMax: 0,  // Maximum Temperature (°C)
  humidityMin: 0,     // Minimum Humidity (%)
  humidityMax: 0,     // Maximum Humidity (%)
  soilMoistureMin: 0, // Minimum Soil Moisture (%)
  soilMoistureMax: 0, // Maximum Soil Moisture (%)
  lightMin: 0,      // Minimum Light Intensity (lux)
  lightMax: 0      // Maximum Light Intensity (lux)
};

// Function to fetch thresholds
function fetchThresholds() {
  fetch('/thresholds') // Replace <esp32-ip> with the actual IP of your ESP32
    .then(response => response.json())
    .then(data => {
      // Map the received data to the thresholds object
      thresholds.temperatureMin = parseFloat(data.tempmin);
      thresholds.temperatureMax = parseFloat(data.tempmax);
      thresholds.humidityMin = parseFloat(data.hummin);
      thresholds.humidityMax = parseFloat(data.hummax);
      thresholds.soilMoistureMin = parseFloat(data.soilmin);
      thresholds.soilMoistureMax = parseFloat(data.soilmax);
      thresholds.lightMin = parseFloat(data.lgtmin);
      thresholds.lightMax = parseFloat(data.lgtmax);

      console.log('Updated thresholds:', thresholds);
    })
    .catch(error => {
      console.error('Error fetching thresholds:', error);
    });
}

// Function to show devices
function showDevices() {
  fetch('/slavedevices')
    .then(response => response.json())
    .then(data => {
      const devicesContainer = document.getElementById('device-buttons-container');
      devicesContainer.innerHTML = ''; // Clear existing buttons
      document.getElementById("change-display-name-btn").style.display = "inline-block";

      if (data.devices && data.devices.length > 0) {
        data.devices.forEach(device => {
          const button = document.createElement('button');
          button.classList.add('device-button');
          button.textContent = device;
          button.addEventListener('click', function () {
            handleDeviceClick(device, button);
          });
          devicesContainer.appendChild(button);
        });
      } else {
        devicesContainer.innerHTML = '<p>No devices available.</p>';
      }
      console.log('Device names:', data);
    })
    .catch(error => {
      console.error('Error fetching devices:', error);
      const devicesContainer = document.getElementById('device-buttons-container');
      devicesContainer.innerHTML = '<p>Error fetching devices. Please try again later.</p>';
    });
}

// Function to handle device button click
function handleDeviceClick(deviceName, clickedButton) {
  console.log(`Selected device: ${deviceName}`);
  const contentSection = document.getElementById('content');
  contentSection.style.display = 'block';

  const buttons = document.querySelectorAll('.device-button');
  buttons.forEach(button => {
    if (button === clickedButton) {
      button.classList.add('highlighted');
    } else {
      button.classList.remove('highlighted');
    }
  });

  const deviceNameElement = document.getElementById('device-name');
  deviceNameElement.textContent = deviceName;
  document.getElementById('selected-device').style.display = 'block';

  fetch(`/logdevice?name=${encodeURIComponent(deviceName)}`)
    .then(response => {
      if (!response.ok) {
        console.error('Error logging device:', response.statusText);
      }
    })
    .catch(error => console.error('Error sending device name:', error));
}

// Function to show the change name form
function showChangeNameForm() {
  document.getElementById("change-name-form").style.display = "block";
}

// Function to hide the change name form
function hideChangeNameForm() {
  document.getElementById("change-name-form").style.display = "none";
  document.getElementById("current-name").value = "";
  document.getElementById("new-name").value = "";
}

// Function to submit display name change
function submitDisplayNameChange() {
  const currentName = document.getElementById("current-name").value.trim();
  const newName = document.getElementById("new-name").value.trim();

  if (!currentName || !newName) {
    alert("Please fill out both fields!");
    return;
  }

  fetch(`/changedisplayname?currentName=${currentName}&newName=${newName}`)
    .then(response => {
      if (response.ok) {
        alert("Display name updated successfully!");
      } else {
        alert("Failed to update display name. Make sure the current name exists.");
      }
      hideChangeNameForm();
    })
    .catch(error => {
      console.error("Error updating display name:", error);
    });
}

// Function to show graphs
function showGraphs() {
  const graphsSection = document.getElementById('graphs-section');
  graphsSection.style.display = 'block';
  const filteroptions = document.getElementById('filter-options');
  filteroptions.style.display = 'block';
}

// Function to apply filter
function applyFilter() {
  let startDate = document.getElementById("start-date").value;
  let endDate = document.getElementById("end-date").value;

  startDate = formatDate(startDate);
  endDate = formatDate(endDate);

  if (startDate && endDate) {
    let url = `/setFilter?startDate=${startDate}&endDate=${endDate}`;
    fetch(url)
      .then(response => response.json())
      .then(data => {
        console.log('Filter applied:', data);
        updateGraph(temperatureGraph, data.map(d => ({ timestamp: d.D, value: d.T })), 'Temperature (°C)', thresholds.temperatureMin, thresholds.temperatureMax);
        updateGraph(humidityGraph, data.map(d => ({ timestamp: d.D, value: d.H })), 'Humidity (%)', thresholds.humidityMin, thresholds.humidityMax);
        updateGraph(moistureGraph, data.map(d => ({ timestamp: d.D, value: d.M })), 'Soil Moisture (%)', thresholds.soilMoistureMin, thresholds.soilMoistureMax);
        updateGraph(lightGraph, data.map(d => ({ timestamp: d.D, value: d.L })), 'Light Intensity (lux)', thresholds.lightMin, thresholds.lightMax);
      })
      .catch(error => {
        console.error('Error applying filter:', error);
      });
  } else {
    alert("Please select both start and end dates.");
  }
}

// Function to format date
function formatDate(date) {
  const [year, month, day] = date.split("-");
  return `${day}/${month}/${year}`;
}

// Function to reset filters
function resetFilters() {
  document.getElementById("start-date").value = "";
  document.getElementById("end-date").value = "";
  filterdata = null;
  updateGraphs();
  console.log("Filters reset to default values.");
}

// Function to fetch sensor readings
function getReadings() {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      updateGauges(data);
    })
    .catch(error => {
      console.error('Error fetching readings:', error);
    });
}

// Function to update gauges
function updateGauges(data) {
  gaugeTemp.value = data.temperature;
  gaugeHum.value = data.humidity;
  gaugeMoisture.value = data.moisture;
  gaugeLight.value = data.lux;

  gaugeTemp.update();
  gaugeHum.update();
  gaugeMoisture.update();
  gaugeLight.update();
}

// Function to fetch historical data
function fetchSDCardData() {
  return fetch('/historical-data')
    .then(response => {
      if (!response.ok) {
        throw new Error('Failed to fetch data from the server');
      }
      return response.json();
    })
    .catch(error => {
      console.error('Error fetching historical data:', error);
    });
}

// Function to update graphs
function updateGraphs() {
  if (filterdata != null) {
    updateGraph(temperatureGraph, filterdata.map(d => ({ timestamp: d.D, value: d.T })), 'Temperature (°C)', thresholds.temperatureMin, thresholds.temperatureMax);
    updateGraph(humidityGraph, filterdata.map(d => ({ timestamp: d.D, value: d.H })), 'Humidity (%)', thresholds.humidityMin, thresholds.humidityMax);
    updateGraph(moistureGraph, filterdata.map(d => ({ timestamp: d.D, value: d.M })), 'Soil Moisture (%)', thresholds.soilMoistureMin, thresholds.soilMoistureMax);
    updateGraph(lightGraph, filterdata.map(d => ({ timestamp: d.D, value: d.L })), 'Light Intensity (lux)', thresholds.lightMin, thresholds.lightMax);
  } else {
    fetchSDCardData().then(data => {
      updateGraph(temperatureGraph, data.map(d => ({ timestamp: d.D, value: d.T })), 'Temperature (°C)', thresholds.temperatureMin, thresholds.temperatureMax);
      updateGraph(humidityGraph, data.map(d => ({ timestamp: d.D, value: d.H })), 'Humidity (%)', thresholds.humidityMin, thresholds.humidityMax);
      updateGraph(moistureGraph, data.map(d => ({ timestamp: d.D, value: d.M })), 'Soil Moisture (%)', thresholds.soilMoistureMin, thresholds.soilMoistureMax);
      updateGraph(lightGraph, data.map(d => ({ timestamp: d.D, value: d.L })), 'Light Intensity (lux)', thresholds.lightMin, thresholds.lightMax);
    });
  }
}

// Function to update a specific graph
function updateGraph(graph, data, label, thresholdMin, thresholdMax) {
  graph.data.labels = data.map(d => d.timestamp);
  graph.data.datasets[0].data = data.map(d => d.value);
  graph.data.datasets[0].lineTension = 0.4;

  const pointColors = data.map((d) => {
    if (d.value >= thresholdMin && d.value <= thresholdMax) {
      return 'rgba(0, 128, 0, 1)';
    } else {
      return 'rgba(139, 0, 0, 1)';
    }
  });

  graph.data.datasets[0].borderColor = 'rgba(211, 211, 211, 1)';
  graph.data.datasets[0].backgroundColor = pointColors;
  graph.data.datasets[0].fill = false;
  graph.data.datasets[0].borderWidth = 1;
  graph.data.datasets[0].pointRadius = 5;
  graph.data.datasets[0].pointBackgroundColor = pointColors;

  graph.update();
}

// Set interval for updating readings and graphs
setInterval(function () {
  getReadings();
}, 15000);

// Initialize graphs
var temperatureGraph = setupGraph('temperature-graph', 'Temperature (°C)', thresholds.temperatureMin, thresholds.temperatureMax);
var humidityGraph = setupGraph('humidity-graph', 'Humidity (%)', thresholds.humidityMin, thresholds.humidityMax);
var moistureGraph = setupGraph('moisture-graph', 'Soil Moisture (%)', thresholds.soilMoistureMin, thresholds.soilMoistureMax);
var lightGraph = setupGraph('light-graph', 'Light Intensity (lux)', thresholds.lightMin, thresholds.lightMax);

// Function to set up a graph
function setupGraph(canvasId, label, thresholdMin, thresholdMax) {
  return new Chart(document.getElementById(canvasId).getContext('2d'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: label,
        data: [],
        fill: false,
        borderWidth: 2,
      }]
    },
    options: {
      responsive: true,
      plugins: {
        legend: {
          display: true,
          labels: {
            boxWidth: 15,
            color: '#000'
          }
        }
      },
      scales: {
        x: { title: { display: true, text: 'Time' } },
        y: { title: { display: true, text: label } }
      }
    }
  });
}

// Initialize gauges
var gaugeTemp = new RadialGauge({
  renderTo: 'gauge-temperature',
  width: 300,
  height: 300,
  units: "Temperature (°C)",
  minValue: 0,
  maxValue: 50,
  colorPlate: "#fff",
  animationDuration: 1500,
  highlights: [
    { from: 0, to: 10, color: "#00FFFF" },
    { from: 10, to: 30, color: "#006400" },
    { from: 30, to: 40, color: "#DC143C" }
  ],
  majorTicks: [0, 10, 20, 30, 40, 50]
}).draw();

var gaugeHum = new RadialGauge({
  renderTo: 'gauge-humidity',
  width: 300,
  height: 300,
  units: "Humidity (%)",
  minValue: 0,
  maxValue: 100,
  colorPlate: "#fff",
  animationDuration: 1500,
  highlights: [
    { from: 0, to: 30, color: "#00FFFF" },
    { from: 30, to: 60, color: "#006400" },
    { from: 60, to: 100, color: "#DC143C" }
  ]
}).draw();

var gaugeMoisture = new RadialGauge({
  renderTo: 'gauge-moisture',
  width: 300,
  height: 300,
  units: "Soil Moisture (%)",
  minValue: 0,
  maxValue: 100,
  colorPlate: "#fff",
  animationDuration: 1500,
  highlights: [
    { from: 0, to: 30, color: "#DC143C" },
    { from: 30, to: 70, color: "#006400" },
    { from: 70, to: 100, color: "#00FFFF" }
  ]
}).draw();

var gaugeLight = new RadialGauge({
  renderTo: 'gauge-light',
  width: 300,
  height: 300,
  units: "Light (lux)",
  minValue: 0,
  maxValue: 35000,
  colorPlate: "#fff",
  animationDuration: 1500,
  highlights: [
    { from: 0, to: 10760, color: "#00FFFF" },
    { from: 10760, to: 26900, color: "#006400" },
    { from: 26900, to: 35000, color: "#DC143C" }
  ],
  majorTicks: [0, 10760, 26900, 3500]
}).draw();

// Initial call to update the graphs and gauges when the page loads
window.addEventListener('load', function () {
  fetchThresholds();
  getReadings();
  updateGraphs();
  fetchSDCardData();
});

window.onload = function () {
  document.getElementById('start-date').value = '';
  document.getElementById('end-date').value = '';
};

// Add event listener for the filter button
document.getElementById('filter-button').addEventListener('click', function () {
  const startDate = document.getElementById('start-date').value;
  const endDate = document.getElementById('end-date').value;

  if (startDate && endDate) {
    filterGraphsByDate(startDate, endDate);
  } else {
    alert("Please select both start and end dates.");
  }
});