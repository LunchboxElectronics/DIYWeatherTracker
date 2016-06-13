// Webhook for hook.io for DIY Weather Tracker 
// CC BY-SA Teddy Lowe June 2016 - Lunchbox Electronics
// Based on code from monkbroc what to wear https://github.com/monkbroc/what-to-wear

function getForecast(url) {
  const got = require("got"); // got makes it easy to make HTTP requests
  return got(url)
  .then(function (response) {
    return JSON.parse(response.body); // parses out the body of the JSON
  });
}

function hookBack(hook, condition){
  hook.res.end(condition);    // This forces the hook to end and respond with the condition
}

module['exports'] = function getWeather (hook) {
  var location = hook.env.MY_LOCATION;      // This pulls lat/long location from the environment variables 
  var api_key = hook.env.FORECASTIO_API_KEY;  // This pulls the api key for forecast.io from the env vars
  var url = "https://api.forecast.io/forecast/" + api_key + "/" + location; // This generates the URL for the call
  
  getForecast(url) 
  .then(function (weather) {
    var condition = weather.currently.icon;   // All we want is the icon - a machine readable version of the current weather
    console.log(condition);
    hookBack(hook, condition);
  })
  .catch(function (error) {
    console.error("Error getting forecast: " + error);
    hook.res.end("error");
  });
};
