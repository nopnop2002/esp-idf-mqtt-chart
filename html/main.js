//document.getElementById("datetime").innerHTML = "WebSocket is not connected";

var websocket = new WebSocket('ws://'+location.hostname+'/');
var display = false;
var chart1 = 0;
var chart2 = 0;
var chart3 = 0;

var xaxis_min = 0;
var xaxis_max = 100;
var xaxis_index = 100;


function sendText(name) {
	console.log('sendText');
	var data = {};
	data["id"] = name;
	console.log('data=', data);
	json_data = JSON.stringify(data);
	console.log('json_data=' + json_data);
	websocket.send(json_data);
}

websocket.onopen = function(evt) {
	console.log('WebSocket connection opened');
	var data = {};
	data["id"] = "init";
	console.log('data=', data);
	json_data = JSON.stringify(data);
	console.log('json_data=' + json_data);
	websocket.send(json_data);
	//document.getElementById("datetime").innerHTML = "WebSocket is connected!";
}

websocket.onmessage = function(evt) {
	var msg = evt.data;
	console.log("msg=" + msg);
	var values = msg.split('\4'); // \4 is EOT
	//console.log("values=" + values);
	switch(values[0]) {
		case 'HEAD':
			console.log("HEAD values[1]=" + values[1]);
			var h1 = document.getElementById( 'header' );
			h1.textContent = values[1];
			break;

		case 'BROKER':
			console.log("BROKER values[1]=" + values[1]);
			var broker = document.getElementById( 'broker' );
			broker.textContent = values[1];
			break;

		case 'NAME':
			//console.log("gauge1=" + Object.keys(gauge1.options));
			//console.log("gauge1.options.units=" + gauge1.options.units);
			console.log("NAME values[1]=" + values[1]);
			console.log("NAME values[2]=" + values[2]);
			console.log("NAME values[3]=" + values[3]);
			if (values[1] != "") {
				Plotly.restyle('graph', 'visible', true, [0]);
				Plotly.restyle('graph', 'name', values[1], [0])
				chart1 = 1;
			}
			if (values[2] != "") {
				Plotly.restyle('graph', 'visible', true, [1]);
				Plotly.restyle('graph', 'name', values[2], [1])
				chart2 = 1;
			}
			if (values[3] != "") {
				Plotly.restyle('graph', 'visible', true, [2]);
				Plotly.restyle('graph', 'name', values[3], [2])
				chart3 = 1;
			}
			break;

		case 'DATA':
			console.log("DATA values[1]=" + values[1]);
			var data1 = Number(values[1]);
			console.log("DATA data1=" + data1);
			var data2 = 0;
			var data3 = 0;
			if (chart2) {
				console.log("DATA values[2]=" + values[2]);
				data2 = Number(values[2]);
				console.log("DATA data2=" + data2);
			}
			if (chart3) {
				console.log("DATA values[3]=" + values[3]);
				data3 = Number(values[3]);
				console.log("DATA data3=" + data3);
			}

/*
			var time = new Date();
			var update = {
				x: [[time], [time], [time]],
				y: [[data1], [data2], [data3]]
			}

			var olderTime = time.setMinutes(time.getMinutes() - 1);
			var futureTime = time.setMinutes(time.getMinutes() + 1);

			var xaxisView = {
				xaxis: {
					type: 'date',
					range: [olderTime,futureTime]
				}
			};
*/

			var update = {
				x: [[xaxis_index], [xaxis_index], [xaxis_index]],
				y: [[data1], [data2], [data3]]
			}
			xaxis_index++;
			console.log("DATA xaxis_index=" + xaxis_index);

			var xaxisView = {
				xaxis: {
					type: 'linear',
					showticklabels: false,
					range: [xaxis_min,xaxis_max]
				}
			};
			xaxis_min++;
			xaxis_max++;

			Plotly.relayout('graph', xaxisView);
			Plotly.extendTraces('graph', update, [0, 1, 2])

			console.log("DATA display=" + display);
			if (display == false) {
				document.getElementById("graph").style.display = "block";
				display = true;
			}
			break;

		default:
			break;
	}
}

websocket.onclose = function(evt) {
	console.log('Websocket connection closed');
	//document.getElementById("datetime").innerHTML = "WebSocket closed";
}

websocket.onerror = function(evt) {
	console.log('Websocket error: ' + evt);
	//document.getElementById("datetime").innerHTML = "WebSocket error????!!!1!!";
}
