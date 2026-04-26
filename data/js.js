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
var ButtW;
var ButtH;
var butt1=false;
var butt2=false;
var butt3=false;
var butt4=false;
var butt5=false;
var butt6=false;
var butt7=false;
var butt8=false;
var butt9=false;
var butt10=false;
var butt11=false;
var butt12=false;
var butt13=false;
var butt14=false;
var connected=false;
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
    websocket.addEventListener("message",respond)
    websocket.addEventListener("open", (event) => {
    connected=true
    websocket.addEventListener("close",(event)=>{console.log("lost")});
    clearCanv();
    drawDisplay();});
    drawStart();
}

function recon(){
    if (websocket.readyState==3||websocket==null)
    {
        websocket=null;
        websocket=new WebSocket(gateway);
        websocket.addEventListener("message",respond)
        websocket.addEventListener("open", (event) => {
        connected=true
        clearCanv();
        drawDisplay();});
        connected=false;
        clearCanv();
        drawDisplay();
    }
}
setInterval(recon,500);
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
    ButtW=armZone/3;
    ButtH=width/4;
    cur.beginPath();
    cur.arc(circCenter[0],circCenter[1],circRad,0,2*Math.PI);
    // cur.moveTo(width/2,height);
    // cur.lineTo(width/2,height-armZone);
    // cur.arc(arcCenter[0],arcCenter[1],arcRad,Math.PI/2,3*Math.PI/2,true);
    // cur.moveTo(0,height/2);
    // cur.lineTo(width,height/2);
    cur.stroke();
    cur.beginPath();
    if(connected)
    {
        cur.fillStyle = "green";
        cur.strokeStyle="green";
    }
    else
    {
        cur.fillStyle = "red";
        cur.strokeStyle="red";
    }
    cur.arc(0,0,circRad/3,circRad,0,2*Math.PI);
    cur.fill();
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
            //console.log(zone);
            
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
                case "butt7":
                    butt7=true;
                    break;
                case "butt8":
                    butt8=true;
                    break;
                case "butt9":
                    butt9=true;
                    break;
                case "butt10":
                    butt10=true;
                    break;
                case "butt11":
                    butt11=true;
                    break;
                case "butt12":
                    butt12=true;
                    break;
                case "butt14":
                    butt14=true;
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
            var pos=[touch.clientX,touch.clientY];
            var zone=touchMap.get(touch.identifier);
            var endzone=getZone(pos);
            switch (zone)
            {
            case "circ":
                joyPos=[0,0];
                break;
            case "butt1":
                if(zone==endzone)
                {
                    butt1=!butt1;
                }
                break;
            case "butt2":
                if(zone==endzone)
                {
                    butt2=!butt2;
                }
                break;
            case "butt3":
                if(zone==endzone)
                {
                    butt3=!butt3;
                }
                break;
            case "butt4":
                if(zone==endzone)
                {
                    butt4=!butt4;
                }
                break;
            case "butt5":
                if(zone==endzone)
                {
                    butt5=!butt5;
                }
                break;
            case "butt6":
                if(zone==endzone)
                {
                    butt6=!butt6;
                }
                break;
            case "butt7":
                    butt7=false;
                    break;
            case "butt8":
                    butt8=false;
                    break;
            case "butt9":
                    butt9=false;
                    break;
            case "butt10":
                    butt10=false;
                    break;
            case "butt11":
                    butt11=false;
                    break;
            case "butt12":
                    butt12=false;
                    break;
            case "butt13":
                if(zone==endzone)
                {
                    butt13=!butt13;
                }
                break;
            case "butt14":
                if(zone==endzone)
                {
                    butt14=false;
                }
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
    cur.lineWidth=3;
}

function drawDisplay()
{
    drawBackground();
    setupDrawing();
    cur.beginPath();
    cur.arc(circCenter[0]+joyPos[0]*circRad,circCenter[1]+joyPos[1]*circRad,slideZone/2,0,2*Math.PI);
    cur.fill();
    // cur.beginPath();
    // cur.arc(arcCenter[0]+Math.sin(deg2rad(armPos[0]))*armPos[1]*arcRad,arcCenter[1]+Math.cos(deg2rad(armPos[0]))*armPos[1]*arcRad,slideZone/2,0,2*Math.PI);
    // cur.fill();
    // cur.beginPath();
    // cur.rect(slidePos*width-slideZone/4,circZone,slideZone/2,slideZone);
    // cur.fill();
    //this is dumb
    cur.beginPath();
    cur.rect(1,circZone+slideZone,ButtH,ButtW);
    if(butt1)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(1,circZone+slideZone+ButtW,ButtH,ButtW);
    if(butt2)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(1,circZone+slideZone+2*ButtW,ButtH,ButtW);
    if(butt3)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(ButtH,circZone+slideZone,ButtH,ButtW);
    if(butt4)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(ButtH,circZone+slideZone+ButtW,ButtH,ButtW);
        if(butt5)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(ButtH,circZone+slideZone+2*ButtW,ButtH,ButtW);
    if(butt6)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
        cur.beginPath();
    cur.rect(2*ButtH,circZone+slideZone,ButtH,ButtW);
    if(butt7)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(2*ButtH,circZone+slideZone+ButtW,ButtH,ButtW);
        if(butt8)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(2*ButtH,circZone+slideZone+2*ButtW,ButtH,ButtW);
    if(butt9)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(3*ButtH,circZone+slideZone,ButtH,ButtW);
        if(butt10)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(3*ButtH,circZone+slideZone+ButtW,ButtH,ButtW);
        if(butt11)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(3*ButtH,circZone+slideZone+2*ButtW,ButtH,ButtW);
    if(butt12)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(1,circZone,width/2,slideZone);
    if(butt13)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
    cur.beginPath();
    cur.rect(width/2,circZone,width,slideZone);
        if(butt14)
    {
        cur.fill();
    }
    else
    {
        cur.stroke()
    }
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
    // else if (getDist(pos,arcCenter)<=arcRad&&pos[0]>=arcCenter[0]) {
    //     return "arc"
    // }
    // else if (pos[1]>=circZone&&pos[1]<=circZone+slideZone)
    // {
    //     return "slide"
    // }
    else if (pos[0]<ButtH&&pos[1]>circZone+slideZone&&pos[1]<circZone+slideZone+ButtW)
    {
        return "butt1"
    }
        else if (pos[0]<ButtH&&pos[1]>circZone+slideZone+ButtW&&pos[1]<circZone+slideZone+2*ButtW)
    {
        return "butt2"
    }
            else if (pos[0]<ButtH&&pos[1]>circZone+slideZone+2*ButtW)
    {
        return "butt3"
    }
        else if (pos[0]>ButtH&&pos[0]<2*ButtH&&pos[1]>circZone+slideZone&&pos[1]<circZone+slideZone+ButtW)
    {
        return "butt4"
    }
        else if (pos[0]>ButtH&&pos[0]<2*ButtH&&pos[1]>circZone+slideZone+ButtW&&pos[1]<circZone+slideZone+2*ButtW)
    {
        return "butt5"
    }
        else if (pos[0]>ButtH&&pos[0]<2*ButtH&&pos[1]>circZone+slideZone+2*ButtW)
    {
        return "butt6"
    }
        else if (pos[0]>2*ButtH&&pos[0]<3*ButtH&&pos[1]>circZone+slideZone&&pos[1]<circZone+slideZone+ButtW)
    {
        return "butt7"
    }
        else if (pos[0]>2*ButtH&&pos[0]<3*ButtH&&pos[1]>circZone+slideZone+ButtW&&pos[1]<circZone+slideZone+2*ButtW)
    {
        return "butt8"
    }
        else if (pos[0]>2*ButtH&&pos[0]<3*ButtH&&pos[1]>circZone+slideZone+2*ButtW)
    {
        return "butt9"
    }
        else if (pos[0]>3*ButtH&&pos[0]<4*ButtH&&pos[1]>circZone+slideZone&&pos[1]<circZone+slideZone+ButtW)
    {
        return "butt10"
    }
        else if (pos[0]>3*ButtH&&pos[0]<4*ButtH&&pos[1]>circZone+slideZone+ButtW&&pos[1]<circZone+slideZone+2*ButtW)
    {
        return "butt11"
    }
        else if (pos[0]>3*ButtH&&pos[0]<4*ButtH&&pos[1]>circZone+slideZone+2*ButtW)
    {
        return "butt12"
    }
        else if (pos[0]<=width/2&&pos[1]>circZone&&pos[1]<circZone+slideZone)
    {
        return "butt13"
    }
        else if (pos[0]>width/2&&pos[1]>circZone&&pos[1]<circZone+slideZone)
    {
        return "butt14"
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
            websocket.send([
                joyPos[0],
                joyPos[1],
                armPos[0],
                armPos[1],
                slidePos,
                Number(butt1),
                Number(butt2),
                Number(butt3),
                Number(butt4),
                Number(butt5),
                Number(butt6),
                Number(butt7),
                Number(butt8),
                Number(butt9),
                Number(butt10),
                Number(butt11),
                Number(butt12),
                Number(butt13),
                Number(butt14)+','
            ].join(","));
            break;
        default:
            console.log("unknown comm");
            break;
    }
}