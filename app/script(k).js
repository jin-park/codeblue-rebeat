function updateTime() {
    const timeElement = document.getElementById('time');
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    timeElement.textContent = `Current Time: ${timeString}`;
}

function updateSensorStatus(isNear) {
    const sensorStatusElement = document.getElementById('sensor-status');
    sensorStatusElement.textContent = `Proximity Sensor Status: ${isNear ? 'Near' : 'Far'}`;
}

function handleSensorChange(event) {
    const isNear = event.near;
    updateSensorStatus(isNear);
}

window.addEventListener('DOMContentLoaded', () => {
    updateTime();
    
    // Update the time every second
    setInterval(updateTime, 1000); 

    updateSensorStatus(false); // Initialize as "Far"

   // Check for landscape mode
   if (window.innerWidth > window.innerHeight) {
        window.addEventListener('resize', () => {
            if (window.innerWidth < window.innerHeight) {
                // Reload the page if the device is no longer in landscape mode
                location.reload();
            }
        });

        // Proximity sensor
        if ('ondeviceproximity' in window) {
            window.addEventListener('deviceproximity', handleSensorChange);
        }
   } else {
       alert('Please rotate your device to landscape mode.');
   }
});
