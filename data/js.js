var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var canvas;
var cur;
var width;
var height;
var slideZone;
var circZone;
var armZone;
var fullscreen=false;
var state;
var joyPos;
var slidePos;
var armPos;

window.addEventListener("load",onLoad);
document.addEventListener("fullscreenchange",swap);
document.addEventListener("touchstart",handStart);
document.addEventListener("touchend",handEnd);
document.addEventListener("touchmove",handMove);

function onLoad(event)
{
    websocket=new WebSocket(gateway);

    //setup drawing
    //note: everything is drawn sideways, don't want to bother with screenlock stuff
    canvas=document.getElementById("draw");
    width=document.body.clientWidth;
    height=document.body.clientHeight;
    canvas.width=width;
    canvas.height=height;
    cur=canvas.getContext("2d");
    cur.fillStyle = "white";
    cur.strokeStyle="white";
    drawStart();
}



/*document.onclick=function(){
    
    //websocket.send(1);
}*/


function swap(event){
    fullscreen=!fullscreen;

    width=document.body.clientWidth;
    height=document.body.clientHeight;
    canvas.width=width;
    canvas.height=height;

    if (fullscreen)
    {
        clearCanv();
        drawBackground();
    }
    else
    {
        clearCanv();
        drawStart();
    }
}

function drawBackground()
{
    cur.fillStyle = "white";
    cur.strokeStyle="white";

    circZone=.45*height;
    slideZone=.1*height;
    armZone=.45*height;

    cur.beginPath();
    cur.arc(width/2,circZone/2,circZone/2,0,2*Math.PI);
    cur.moveTo(width/2,height);
    cur.lineTo(width/2,height-armZone);
    cur.arc(width/2,height-armZone/2,armZone/2,Math.PI/2,3*Math.PI/2,true);
    cur.moveTo(0,height/2);
    cur.lineTo(width,height/2);
    cur.stroke();
}

function drawStart(event)
{
    cur.fillStyle = "white";
    cur.strokeStyle="white";

    cur.textAlign="center";
    cur.textBaseline="middle";
    cur.font="100px Verdana";
    cur.fillText("Touch!",width/2,height/2,width);
}

function clearCanv()
{
    cur.clearRect(0,0,canvas.width,canvas.height);
}

function handStart(event)
{
    if (fullscreen)
    {
        
    }
}

function handEnd(event)
{
    if (!fullscreen)
    {
        document.documentElement.requestFullscreen();
    }
}

function handMove(event)
{

}