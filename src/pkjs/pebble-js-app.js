var myAPIKey = 'fd541b64f6da378322b01f9a122d5dee';
var watchID;

function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function distance(lat1, lon1, lat2, lon2, unit) {
	var radlat1 = Math.PI * lat1/180;
	var radlat2 = Math.PI * lat2/180;
	var theta = lon1-lon2;
	var radtheta = Math.PI * theta/180;
	var dist = Math.sin(radlat1) * Math.sin(radlat2) + Math.cos(radlat1) * Math.cos(radlat2) * Math.cos(radtheta);
	dist = Math.acos(dist);
	dist = dist * 180/Math.PI;
	dist = dist * 60 * 1.1515;
	if (unit=="K") { dist = dist * 1.609344; }
	if (unit=="N") { dist = dist * 0.8684; }
  console.log("Distance:");
	console.log(dist);
  return dist.toFixed(2) + ' miles';
}

function fetchWeather(latitude, longitude) {
  //latitude = '51.400663';
  //longitude = '-0.259263';
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = Math.round(response.main.temp - 273.15);
        var icon = iconFromWeatherId(response.weather[0].id);
        var city = response.name;
        console.log(temperature);
        console.log(icon);
        console.log(city);
        var dist = distance(latitude, longitude, 51.40065, -0.2617,"M");
        console.log(dist);
        Pebble.sendAppMessage({
          'WEATHER_ICON_KEY': icon,
          'WEATHER_TEMPERATURE_KEY': temperature + '\xB0C',
          'WEATHER_CITY_KEY': city,
          'DISTANCE_KEY': dist
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

/*
function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
  console.log('Fetched weather');
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'WEATHER_CITY_KEY': 'Loc Unavailable',
    'WEATHER_TEMPERATURE_KEY': 'N/A'
  });
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};
*/
var options = {
  enableHighAccuracy: true,
  maximumAge: 0,
  timeout: 5000
};

function success(pos) {
  var coords = pos.coords;
  console.log('Position changed handler');
  console.log(coords);
  fetchWeather(coords.latitude, coords.longitude);
  
}

function error(err) {
  console.warn('location error' + err.code + '): ' + err.message);
}

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  //navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    //locationOptions);
  // Get location updates
  watchID = navigator.geolocation.getCurrentPosition(success, error, options);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  console.log(e.type);
  console.log(JSON.stringify(e.payload));
  var dict = e.payload;
  
  if (dict['4'])
  {
    var value = dict['4'];
    console.log(value);
    // fetchWeather(); ???
  }
});

/*Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
}); */