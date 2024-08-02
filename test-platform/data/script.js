const LOCALSTORAGE_KEY_LC_HISTORY = "LCHistory";
const LOCALSTORAGE_KEY_PR_HISTORY = "PRHistory";

// loadcell
let readCount = document.getElementById("readCount");
let readValue = document.getElementById("value");
let readIndicator = document.getElementById("readIndicator");
let scaleValueInput = document.getElementById("scaleValue");
let tareButton = document.getElementById("tareButton");
let calibrateButton = document.getElementById("calibrateButton");
let readIndicatorValue = 1;

// pressure sensor
let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    // setTimeout(loadCellRead, 1000);
    loadCellReadScale();
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 1000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    let myObj;
    try {
        myObj = JSON.parse(event.data);
    }catch {
        console.warn("MALFORMED input");
        return;
    }

    if(myObj["Type"] == "Pressure") {
        onPressureReading(myObj);
    }
    else if(myObj["Type"] == "LoadCell") {
        onLoadCellReading(myObj);
    }
    else {
        console.log("unknown packet");
    }
}

function onLoadCellReading(myObj) {
    console.log("load cell input");
    let value = myObj["Value"]
    readValue.innerText = value;
    let storage = JSON.parse(localStorage.getItem(LOCALSTORAGE_KEY_LC_HISTORY) || "{}")
    storage.items.push({
        timestamp: Date.now(),
        value
    });
    localStorage.setItem(LOCALSTORAGE_KEY_LC_HISTORY, JSON.stringify(storage));
}

function onPressureReading(myObj) {
    console.log("pressure input");
    var keys = Object.keys(myObj);

    let storage = JSON.parse(localStorage.getItem(LOCALSTORAGE_KEY_LC_HISTORY) || "{}")

    let item = {
        timestamp: Date.now()
    };

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        item[key] = myObj[key];  
        document.getElementById(key).innerHTML = myObj[key];
    }

    storage.items.push(item);
    localStorage.setItem(LOCALSTORAGE_KEY_LC_HISTORY, JSON.stringify(storage));
}

// function loadCellRead() {
//     readCountValue = readCount.value;
//     readIndicatorValue = (readIndicatorValue + 1) % 5
//     readIndicator.innerText = "o".repeat(readIndicatorValue) + ".";
//     fetch("/get?count="+readCountValue)
//         .then(response => response.text())
//         .then(response => {
//             readValue.innerText = response;
//             readIndicator.innerText = "o".repeat(readIndicatorValue) + "o";
//             setTimeout(loadCellRead, 500);
//         })
//         .catch(er => {
//             console.log(er);
//             console.log("start read again with `read()`");
//             readIndicator.innerText = "err";
//         });
// }

function loadCellTare() {
    tareButton.disabled = true;
    fetch("/tare")
        .then(r => { tareButton.disabled = false})
        .catch(er => {
            console.log(er);
            tareButton.disabled = false
        });
}

function loadCellCalibrate() {
    let value = scaleValueInput.value;
    calibrateButton.disabled = true;
    fetch("/set_scale?scale="+value)
        .then(r => { calibrateButton.disabled = false})
        .catch(er => {
            console.log(er);
            calibrateButton.disabled = false
        });
}

function loadCellReadScale() {
    fetch("/get_scale")
        .then(response => response.text())
        .then(response => {
            scaleValueInput.value = response;
        })
        .catch(er => {
            console.log(er);
        });
}