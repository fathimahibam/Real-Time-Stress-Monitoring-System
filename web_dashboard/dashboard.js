@'
// dashboard.js

console.log("Stress Monitoring Dashboard Started");

let healthData = {
    heartRate: 0,
    spo2: 0,
    temperature: 0,
    gsr: 0
};

function updateDashboard() {

    const heartRateElement = document.getElementById("heart-rate");
    const spo2Element = document.getElementById("spo2");
    const temperatureElement = document.getElementById("temperature");
    const gsrElement = document.getElementById("gsr");

    if (heartRateElement)
        heartRateElement.innerHTML =
            `Heart Rate: ${healthData.heartRate} BPM`;

    if (spo2Element)
        spo2Element.innerHTML =
            `SpO₂: ${healthData.spo2} %`;

    if (temperatureElement)
        temperatureElement.innerHTML =
            `Temperature: ${healthData.temperature} °C`;

    if (gsrElement)
        gsrElement.innerHTML =
            `GSR: ${healthData.gsr}`;
}

setInterval(() => {

    healthData.heartRate =
        Math.floor(Math.random() * 30) + 70;

    healthData.spo2 =
        Math.floor(Math.random() * 5) + 95;

    healthData.temperature =
        (36 + Math.random() * 2).toFixed(1);

    healthData.gsr =
        Math.floor(Math.random() * 1000) + 1000;

    updateDashboard();

}, 2000);

updateDashboard();
'@ | Set-Content web_dashboard\dashboard.js