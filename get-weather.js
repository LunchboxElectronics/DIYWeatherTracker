// Webhook for hook.io for DIY Weather Tracker 
// Teddy Lowe June 2016 - Lunchbox Electronics
// Based on code from monkbroc what to wear https://github.com/monkbroc/what-to-wear

function getForecast(url) {
  const got = require("got");
  return got(url)
  .then(function (response) {
    return JSON.parse(response.body);
  });
}

function hookBack(hook, condition){
  hook.res.end(condition);
}

module['exports'] = function getWeather (hook) {
  var location = hook.env.MY_LOCATION;
  var api_key = hook.env.FORECASTIO_API_KEY;
  var url = "https://api.forecast.io/forecast/" + api_key + "/" + location;
  
  getForecast(url)
  .then(function (weather) {
    var condition = weather.icon;
    console.log(condition);
    hookBack(hook, condition);
  })
  .catch(function (error) {
    console.error("Error getting forecast: " + error);
    hook.res.end("error");
  });
};
