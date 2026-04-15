function fetchSensorData() {
    fetch('/data?t=' + Date.now())
        .then(res => res.json())
        .then(data => {
            document.getElementById("volume").innerText = data.volume + " %";
            document.getElementById("rate").innerText = data.rate + " ml/min";
            
            // Format time: if it's seconds, maybe convert to min:sec
            let sec = parseInt(data.time);
            let displayTime = sec > 60 ? Math.floor(sec/60) + "m " + (sec%60) + "s" : sec + "s";
            document.getElementById("time").innerText = displayTime;

            const statusEl = document.getElementById("status");
            statusEl.innerText = data.status;

            // Alert Colors
            if (data.status.includes("CRITICAL") || data.status.includes("BACKFLOW")) {
                statusEl.style.color = "#ff4d4d";
            } else if (data.status.includes("LOW") || data.status.includes("BLOCKED")) {
                statusEl.style.color = "orange";
            } else if (data.status.includes("SETUP")) {
                statusEl.style.color = "#00d4ff";
            } else {
                statusEl.style.color = "#4dff88";
            }
        });
}

setInterval(fetchSensorData, 2000);