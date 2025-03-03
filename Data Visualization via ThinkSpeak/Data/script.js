// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

// Add an event listener for the filter button
document.getElementById('filter-button').addEventListener('click', function () {
  const startDate = document.getElementById('start-date').value;
  const endDate = document.getElementById('end-date').value;

  if (startDate && endDate) {
    filterGraphsByDate(startDate, endDate);
  } else {
    alert("Please select both start and end dates.");
  }
});


// Create Temperature Gauge (Radial)
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
    { from: 0, to: 10, color: "#00FFFF" },    // Light Blue for cold
    { from: 10, to: 30, color: "#006400" },  // Light Green for optimal
    { from: 30, to: 40, color: "#DC143C" }   // Light Red for hot
  ],
  majorTicks : [0, 10, 20, 30, 40,50]
}).draw();

// Create Humidity Gauge (Radial)
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
    { from: 0, to: 30, color: "#00FFFF" },    // Light Blue for low humidity
    { from: 30, to: 60, color: "#006400" },  // Light Green for optimal
    { from: 60, to: 100, color: "#DC143C" }  // Light Red for high humidity
  ]
}).draw();

// Create Soil Moisture Gauge (Radial)
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
    { from: 0, to: 30, color: "#DC143C" },    // Light Red for dry soil
    { from: 30, to: 70, color: "#006400" },   // Light Green for optimal moisture
    { from: 70, to: 100, color: "#00FFFF" }   // Light Blue for wet soil
  ]
}).draw();

// Create Light Intensity Gauge (Radial)
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
    { from: 0, to: 10760, color: "#00FFFF" },    // Light Blue for low light
    { from: 10760, to: 26900, color: "#006400" }, // Light Green for optimal light
    { from: 26900, to: 35000, color: "#DC143C" }  // Light Red for too much bright light
  ],
  majorTicks: ['0', '10760', '26900', '35000'],  // Only major ticks at these values
  minorTicks: 10, // Minor ticks between major ticks
  strokeTicks: true,
  ticksAngle: 270, // Full circular gauge angle
  startAngle: 45, // Start position for gauge
  fontNumbersSize: 14,
  colorNumbers: "#000", // Numbers in black for better visibility
  colorNeedle: "#000",
  colorNeedleEnd: "#DC143C",
  needleWidth: 2,
  needleCircleOuter: true,
  needleCircleOuterWidth: 3,
  borders: false // Remove outer border for a clean look
}).draw();




// Initialize the ThingSpeak API URLs
const channelID = '2764096';
const apiKey = 'B6B3SRQLIX67Q5SF';

// Function to fetch all data from ThingSpeak for each field
function fetchThingSpeakData(field) {
  const url = `https://api.thingspeak.com/channels/${channelID}/fields/${field}.json?api_key=${apiKey}&results=300`; // Fetch last 100 data points

  return fetch(url)
    .then(response => response.json())
    .then(data => {
      return data.feeds.map(feed => {
        return {
          timestamp: feed.created_at,
          value: feed[`field${field}`]
        };
      });
    });
}

// Function to fetch data from ThingSpeak for a specific field and date range
function fetchFilteredThingSpeakData(field, startDate, endDate) {
  const url = `https://api.thingspeak.com/channels/${channelID}/fields/${field}.json?api_key=${apiKey}&start=${startDate}&end=${endDate}`;

  return fetch(url)
    .then(response => response.json())
    .then(data => {
      return data.feeds.map(feed => {
        return {
          timestamp: feed.created_at,
          value: feed[`field${field}`]
        };
      });
    });
}


// Function to get the latest readings from ESP32 server for the gauges
function getReadings() {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      updateGauges(data);
    });
}

// Update the gauges with real-time data
function updateGauges(data) {
  gaugeTemp.value = data.temperature;
  gaugeHum.value = data.humidity;
  gaugeMoisture.value = data.moisture;
  gaugeLight.value = data.lux;

  // Update the gauges dynamically
  gaugeTemp.update();
  gaugeHum.update();
  gaugeMoisture.update();
  gaugeLight.update();
}

// Update Graphs function to fetch data from ThingSpeak and update charts
function updateGraphs() {
  Promise.all([
    fetchThingSpeakData(1), // Temperature
    fetchThingSpeakData(2), // Humidity
    fetchThingSpeakData(3), // Light Intensity
    fetchThingSpeakData(4)  // Soil Moisture
  ]).then(([temperatureData, humidityData, lightData, moistureData]) => {
    updateGraph(temperatureGraph, temperatureData, 'Temperature (°C)', 'rgba(85, 85, 85, 1)');
    updateGraph(humidityGraph, humidityData, 'Humidity (%)', 'rgba(54, 162, 235, 1)');
    updateGraph(lightGraph, lightData, 'Light Intensity (lux)', 'rgba(255, 206, 86, 1)');
    updateGraph(moistureGraph, moistureData, 'Soil Moisture (%)', 'rgba(75, 192, 192, 1)');
  });
}

// Function to filter and update graphs based on the selected date range
function filterGraphsByDate(startDate, endDate) {
  Promise.all([
    fetchFilteredThingSpeakData(1, startDate, endDate), // Temperature
    fetchFilteredThingSpeakData(2, startDate, endDate), // Humidity
    fetchFilteredThingSpeakData(3, startDate, endDate), // Light Intensity
    fetchFilteredThingSpeakData(4, startDate, endDate)  // Soil Moisture
  ]).then(([temperatureData, humidityData, lightData, moistureData]) => {
    updateGraph(temperatureGraph, temperatureData, 'Temperature (°C)', 'rgba(255, 99, 132, 1)');
    updateGraph(humidityGraph, humidityData, 'Humidity (%)', 'rgba(54, 162, 235, 1)');
    updateGraph(lightGraph, lightData, 'Light Intensity (lux)', 'rgba(255, 206, 86, 1)');
    updateGraph(moistureGraph, moistureData, 'Soil Moisture (%)', 'rgba(75, 192, 192, 1)');
  });
}


// Function to update the graph with new data
function updateGraph(graph, data, label, borderColor) {
  graph.data.labels = [];
  graph.data.datasets[0].data = [];
  graph.data.datasets[0].borderColor = borderColor;

  data.forEach(entry => {
    graph.data.labels.push(entry.timestamp);
    graph.data.datasets[0].data.push(entry.value);
  });

  graph.update();
}

// Set the graph and gauge refresh interval to 10 seconds
setInterval(function() {
  // Update gauges from the ESP32 sensor data
  getReadings();
  
  // Update the graphs from ThingSpeak
  updateGraphs();
}, 10000); // Update every 10 seconds

// Initialize Graphs (Temperature, Humidity, Soil Moisture, Light Intensity)
var temperatureGraph = setupGraph('temperature-graph', 'Temperature (°C)');
var humidityGraph = setupGraph('humidity-graph', 'Humidity (%)');
var moistureGraph = setupGraph('moisture-graph', 'Soil Moisture (%)');
var lightGraph = setupGraph('light-graph', 'Light Intensity (lux)');


// Initialize Graphs with Chart.js and different colors
function setupGraph(canvasId, label) {
  return new Chart(document.getElementById(canvasId).getContext('2d'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: label,
        data: [],
        borderColor: '',
        borderWidth: 2,
        tension: 0.4,
        fill: false
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

// Initial call to update the graphs and gauges when the page loads
window.addEventListener('load', function() {
  getReadings(); // Get the latest readings from ESP32
  updateGraphs(); // Get the latest data from ThingSpeak
});



