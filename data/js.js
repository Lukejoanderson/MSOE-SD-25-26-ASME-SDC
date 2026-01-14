var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener("load",onLoad);

function onLoad(event)
{
    websocket=new WebSocket(gateway);
}

document.onclick=function(){
    websocket.send(1);
}