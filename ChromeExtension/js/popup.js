var pluginObj;
var timer;
var count=0;

function print(msg){
  $('div').append( msg )   
}

function callback(result){
  result = JSON.parse(result);
  print('<p class="npp"> [ NPP ]: '+ result['msg']+'</p>') 
  clearInterval(timer);
}

document.addEventListener('DOMContentLoaded', function () {
  pluginObj = document.getElementById("pluginObj");
  if (typeof pluginObj == "object") {
    pluginObj.callback = callback;
    print('<p class="js"> [ JS ]: I\'m calling Plugin function!</p>') 
    var msg = pluginObj.function();
    print('<p class="npp"> [ NPP ]: '+ msg+'</p>') 
  } 
  timer = setInterval(function(){count++; print('<p class="js"> [ JS ]: Hey, I\'ve been waiting '+500*count+' ms...')}, 500);
}, false);
