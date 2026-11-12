
let selectedAlgorithm = "fixed";
const API_BASE_URL = "https://distributed-ratelimiter.netlify.app";
let totalRequests = 0;
let allowedRequests = 0;
let deniedRequests = 0;

const algorithmNames = {
    fixed: "Fixed Window",
    sliding: "Sliding Window",
    token: "Token Bucket",
    leaky: "Leaky Bucket"
};

const algorithmRoutes = {
    fixed: "/count/",
    sliding: "/sliding/",
    token: "/token/",
    leaky: "/leaky/"
};

// Select an algorithm
const algorithmCards = document.querySelectorAll(".algorithm-card");

algorithmCards.forEach(card => {
    card.addEventListener("click", () => {

        algorithmCards.forEach(item => {
            item.classList.remove("selected");
        });

        card.classList.add("selected");

        selectedAlgorithm = card.dataset.algorithm;

        document.getElementById("selectedAlgorithm").textContent =
            algorithmNames[selectedAlgorithm];
    });
});

// Check server status
async function checkServer() {
    const status = document.getElementById("serverStatus");

    try {
        const response = await fetch("/");
        
        if (response.ok) {
            status.textContent = "Online";
            status.style.color = "#4ade80";
        } else {
            status.textContent = "Error";
        }

    } catch (error) {
        status.textContent = "Offline";
        status.style.color = "#f87171";
    }
}

// Send one request
async function sendRequest() {

    const clientId = document.getElementById("clientId").value.trim();
    const result = document.getElementById("requestResult");

    if (!clientId) {
        result.textContent = "Please enter a Client ID.";
        return;
    }

    const route = algorithmRoutes[selectedAlgorithm] +
                  encodeURIComponent(clientId);

    try {
        const response = await fetch(route);
        const message = await response.text();

        totalRequests++;

        if (response.status === 200) {
            allowedRequests++;
            result.textContent = "Request Allowed: " + message;
            result.style.color = "#4ade80";
        } else if (response.status === 429) {
            deniedRequests++;
            result.textContent = "Request Denied: " + message;
            result.style.color = "#f87171";
        } else {
            result.textContent = "Server error: " + message;
            result.style.color = "#facc15";
        }

        updateStats();
        addHistory(clientId, algorithmNames[selectedAlgorithm],
                   response.status === 200 ? "Allowed" :
                   response.status === 429 ? "Denied" : "Error");

    } catch (error) {
        result.textContent = "Could not connect to the server.";
        result.style.color = "#f87171";
    }
}

// Send 10 requests one by one
async function sendMultipleRequests() {

    const result = document.getElementById("requestResult");

    result.textContent = "Sending 10 requests...";

    for (let i = 0; i < 10; i++) {
        await sendRequest();
    }
}

// Update statistics
function updateStats() {
    document.getElementById("totalRequests").textContent = totalRequests;
    document.getElementById("allowedRequests").textContent = allowedRequests;
    document.getElementById("deniedRequests").textContent = deniedRequests;
}

// Add request to history
function addHistory(clientId, algorithm, status) {

    const historyBody = document.getElementById("historyBody");

    if (totalRequests === 1) {
        historyBody.innerHTML = "";
    }

    const row = document.createElement("tr");

    const values = [
        clientId,
        algorithm,
        status,
        new Date().toLocaleTimeString()
    ];

    values.forEach(value => {
        const cell = document.createElement("td");
        cell.textContent = value;
        row.appendChild(cell);
    });

    historyBody.prepend(row);
}

// Button events
document.getElementById("sendRequest").addEventListener("click", sendRequest);

document.getElementById("sendMultiple").addEventListener("click", sendMultipleRequests);

// Initial setup
checkServer();
