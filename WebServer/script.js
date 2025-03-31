//Establish a WebSocket connection
const ws = new WebSocket(`ws://${window.location.hostname}/ws`);

//Cache DOM elements for easy updates
const timeElement = document.getElementById("Time");
const wifiElement = document.getElementById("WiFi");
const autoModeElement = document.getElementById("AutoMode");
const tartalyElement = document.getElementById("Tartaly");
const solTartElement = document.getElementById("SolTart");


//Update functions for each type
function updateTime(time) {
  timeElement.textContent = time;
}

function updateWiFi(color = 'white') {
  // Remove existing arcs (but leave the dot created by ::before)
  wifiElement.innerHTML = '';

  // Create and append 3 arcs
  for (let i = 1; i <= 4; i++) {
    const arc = document.createElement('span');
    arc.classList.add('arc', `arc${i}`);
    arc.style.borderColor = `${color} transparent transparent transparent`; // Update color dynamically
    wifiElement.appendChild(arc);
  }
}

function updateAutoMode(autoMode) {
  if (autoMode) {
    autoModeElement.className = "true";
    autoModeElement.textContent = "A";
  } else {
    autoModeElement.className = "false";
    autoModeElement.textContent = "A";
  }
}

function updateTartaly(tartaly) {
  if (tartaly) {
    tartalyElement.className = "true";
    tartalyElement.textContent = "V";
  } else {
    tartalyElement.className = "false";
    tartalyElement.textContent = "V";
  }
}

function updateSolTart(solTartArray) {
  solTartElement.innerHTML = "";  // Clear existing content

  solTartArray.forEach((val, index) => {
    const solTartItem = document.createElement("span");  // Create span for each element
    solTartItem.textContent = index + 1;  // Always show the number (1-4)

    // Apply appropriate class for true/false
    if (val) {
      solTartItem.className = "true";  // Apply 'true' class if value is true
    } else {
      solTartItem.className = "false";  // Apply 'false' class if value is false
    }

    solTartElement.appendChild(solTartItem);  // Append to the parent element
  });
}


//Handle incoming WebSocket messages
ws.onmessage = (event) => {
  const data = JSON.parse(event.data);

  switch (data.type) {
    
    case "Time":
      updateTime(data.Time);
    break;

    case "WiFiStatus":
      updateWiFi(data.WiFiStatus);
    break;

    case "AutoMode":
      updateAutoMode(data.AutoMode);
    break;

    case "Tartaly":
      updateTartaly(data.Tartaly);
    break;

    case "SolTart":
      updateSolTart(data.SolTart);
    break;

    default:
      console.warn("Unknown message type:", data.type);
    break;

  }

};


// Handle WebSocket errors
ws.onerror = (error) => {
  console.error("WebSocket error:", error);
};

// Reconnect on WebSocket close
ws.onclose = () => {
  console.warn("WebSocket closed. Reconnecting...");
  setTimeout(() => {
    window.location.reload();
  }, 1000);
};