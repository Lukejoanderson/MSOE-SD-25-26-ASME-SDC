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
var joyPos=[0,0];
var slidePos=.5;
var armPos=[90,0];
var circCenter;
var arcCenter;
var circRad;
var arcRad;
var touchMap= new Map();

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
    drawStart();
    websocket.addEventListener("message",respond)
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
        drawDisplay();
    }
    else
    {
        clearCanv();
        drawStart();
    }
}

function drawBackground()
{
    setupDrawing();
    circZone=.45*height;
    slideZone=.1*height;
    armZone=.45*height;

    circCenter=[width/2,circZone/2];
    circRad=circZone/2;
    arcCenter=[width/2,height-armZone/2];
    arcRad=armZone/2;

    cur.beginPath();
    cur.arc(circCenter[0],circCenter[1],circRad,0,2*Math.PI);
    cur.moveTo(width/2,height);
    cur.lineTo(width/2,height-armZone);
    cur.arc(arcCenter[0],arcCenter[1],arcRad,Math.PI/2,3*Math.PI/2,true);
    cur.moveTo(0,height/2);
    cur.lineTo(width,height/2);
    cur.stroke();
}

function drawStart(event)
{
    setupDrawing();
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
        for (var i=0; i<event.changedTouches.length; i++)
        {
            var touch=event.changedTouches[i];
            var pos=[touch.clientX,touch.clientY];
            var zone=getZone(pos);
            touchMap.set(touch.identifier,zone);
            switch (zone)
            {
                case "circ":
                    handCirc(pos);
                    break;
                case "arc":
                    handArc(pos);
                    break;
                case "slide":
                    handSlide(pos);
                    break;
                default:
                    break;
            }
            
        }
        clearCanv();
        drawDisplay();
    }
}

function handEnd(event)
{
    if (!fullscreen)
    {
        document.documentElement.requestFullscreen();
    }
    else
    {
        for (var i=0; i<event.changedTouches.length; i++)
        {
            var touch=event.changedTouches[i];
            var zone=touchMap.get(touch.identifier);
            switch (zone)
            {
            case "circ":
                joyPos=[0,0];
                break;
            default:
                break;
            }
                    
        }
        clearCanv();
        drawDisplay();
    }
}

function handMove(event)
{
    if (fullscreen)
    {
        for (var i=0; i<event.changedTouches.length; i++)
        {
            var touch=event.changedTouches[i];
            var pos=[touch.clientX,touch.clientY];
            var zone=touchMap.get(touch.identifier);
            switch (zone)
            {
            case "circ":
                handCirc(pos);
                break;
            case "arc":
                handArc(pos);
                break;
            case "slide":
                handSlide(pos);
                break;
            default:
                break;
            }
                    
        }
        clearCanv();
        drawDisplay();
    }
}

function setupDrawing()
{
     cur.fillStyle = "white";
    cur.strokeStyle="white";
}

function drawDisplay()
{
    drawBackground();
    setupDrawing();
    cur.beginPath();
    cur.arc(circCenter[0]+joyPos[0]*circRad,circCenter[1]+joyPos[1]*circRad,slideZone/2,0,2*Math.PI);
    cur.fill();
    cur.beginPath();
    cur.arc(arcCenter[0]+Math.sin(deg2rad(armPos[0]))*armPos[1]*arcRad,arcCenter[1]+Math.cos(deg2rad(armPos[0]))*armPos[1]*arcRad,slideZone/2,0,2*Math.PI);
    cur.fill();
    cur.beginPath();
    cur.rect(slidePos*width-slideZone/4,circZone,slideZone/2,slideZone);
    cur.fill();
    cur.stroke();

}

function deg2rad(deg)
{
    return deg*Math.PI/180;
}

function getZone(pos)
{
    if (getDist(pos,circCenter)<=circRad)
    {
        return "circ"
    }
    else if (getDist(pos,arcCenter)<=arcRad&&pos[0]>=arcCenter[0]) {
        return "arc"
    }
    else if (pos[1]>=circZone&&pos[1]<=circZone+slideZone)
    {
        return "slide"
    }
    else 
    {
        return "none"
    }
}

function getDist(pos1,pos2)
{
    return Math.sqrt((pos1[0]-pos2[0])**2+(pos1[1]-pos2[1])**2)
}

function handCirc(pos)
{
    if (getDist(pos,circCenter)<=circRad)
    {
        joyPos=[(pos[0]-circCenter[0])/circRad,(pos[1]-circCenter[1])/circRad];
    }
    else
    {
        var angle=Math.atan((pos[1]-circCenter[1])/(pos[0]-circCenter[0]));
        if (pos[0]<circCenter[0])
        {
            angle=angle+Math.PI;
        }
        //console.log(angle);
        joyPos=[Math.cos(angle),Math.sin(angle)];
    }
}

function handSlide(pos)
{
    slidePos=clamp(pos[0]/width,0,1);
}

function handArc(pos)
{
    if (getDist(pos,arcCenter)<=arcRad&&pos[0]>=arcCenter[0])
    {
        var angle=rad2deg(Math.atan((pos[0]-arcCenter[0])/(pos[1]-arcCenter[1])));
        if(pos[1]<arcCenter[1])
        {
            angle=180-rad2deg(Math.atan((pos[0]-arcCenter[0])/(arcCenter[1]-pos[1])));
        }
        armPos=[angle,getDist(pos,arcCenter)/arcRad];
    }
    else if (pos[0]<arcCenter[0])
    {
        var dist=0;
        if (pos[1]<arcCenter[1])
        {
            angle=180;
            dist=clamp((arcCenter[1]-pos[1])/arcRad,0,1);
        }
        else
        {
            angle=0;
            dist=clamp((pos[1]-arcCenter[1])/arcRad,0,1);
        }
        armPos=[angle,dist];
    }
    else
    {
        var angle=rad2deg(Math.atan((pos[0]-arcCenter[0])/(pos[1]-arcCenter[1])));
        if(pos[1]<arcCenter[1])
        {
            angle=180-rad2deg(Math.atan((pos[0]-arcCenter[0])/(arcCenter[1]-pos[1])));
        }
        armPos=[angle,1];
    }
}

function rad2deg(rad)
{
    return rad/Math.PI*180;
}

function clamp(num, lower, upper)
{
    return Math.min(Math.max(num, lower), upper);
}

function respond(event)
{
    switch (event.data)
    {
        case "ready":
            websocket.send(joyPos[0]+","+joyPos[1]+","+armPos[0]+","+armPos[1]+","+slidePos+",");
            break;
        default:
            console.log("unknown comm");
            break;
    }
}