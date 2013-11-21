var ctx, canvas;	//canvas
var back_image;
var cars_potition;
var init_setinter, setinter;
var eastCars, southCars, westCars, northCars;
var current_index;
var CrossLightObj;
var jsondata;
var eastCount, southCount, westCount, northCount;
var waitflag;
var speed;

function Init()
{
	canvas = document.getElementById('canvas');
  	ctx = canvas.getContext('2d');
  	back_image = new Image();
  	back_image.src = 'image/background.png';
  	ctx.drawImage(back_image, 0, 0);
  	
  	jsondata=0;
  	
  	eastCars = Array(300);
  	southCars = Array(300);
  	westCars = Array(300);
  	northCars = Array(300);
  	
  	speed = 17;
  	eastCount = southCount = westCount = northCount = 0;
  	
  	CrossLightObj = Array(4);
  	CrossLightObj[0] = new CrossLight(370, 495, 0,360,515,435,460);
  	CrossLightObj[1] = new CrossLight(180, 370, 1,144,362,200,435);
  	CrossLightObj[2] = new CrossLight(170, 180, 2,285,145,205,200);
  	CrossLightObj[3] = new CrossLight(500, 170, 3,520,282,465,205);
  	
  	cars_potition = [[375,700,450,700],[-50,375,-50,450],[300,-50,225,-50],[700,300,700,225]];
  	
  	init_setinter = setinter = 0;
  	current_index = 0; 
  	waitflag =0;
 
}

function ReceiveData()
{
	//receive json data
	var length = 0;
	
	if(init_setinter == 1)
	{
		clearInterval(setinter);
		Init();
	}
	
	$.getJSON('./ori.json',function(data){
		jsondata = data;
		success:changeState();
				init_setinter = 1;
	});  
		
}

function CrossLight(_position_x, _position_y, _state, _signalLeftX, _signalLeftY, _signalRightX, _signalRightY)
{
	this.x = _position_x;
	this.y = _position_y;
	this.signalRightX = _signalRightX;
	this.signalLeftX = _signalLeftX;
	this.signalRightY = _signalRightY;
	this.signalLeftY = _signalLeftY;
	this.state = _state;
	this.CrossLightImg = new Image();
	this.CrossLightImg.src = 'image/signal_board_' + this.state + '.png';
	this.CrossLightSignalLeft = new Image();
	this.CrossLightSignalRight = new Image(); 
	
	CrossLight.prototype.drawCrossLight = function()
	{
		ctx.drawImage(this.CrossLightSignalLeft,this.signalLeftX,this.signalLeftY);
		ctx.drawImage(this.CrossLightSignalRight,this.signalRightX,this.signalRightY);
		ctx.drawImage(this.CrossLightImg,this.x,this.y);
	}
	CrossLight.prototype.setSignalRight = function(_signalLeftX, _signalLeftY, _signalRightX, _signalRightY)
	{
		if(_signalRightX != 0)
			this.signalRightX = _signalRightX;
		if(_signalLeftX != 0)
			this.signalLeftX = _signalLeftX;
		if(_signalRightY != 0)
			this.signalRightY = _signalRightY;
		if(_signalLeftY != 0)
			this.signalLeftY = _signalLeftY;
	}
	
}



function CreateCars(create_index, time)
{
	var i=0, direction = 0;
	var distanceCar = (((speed*time)/100)/jsondata[create_index].count)-40;
 
	if(jsondata[create_index].signal_state == 0)
	{
		direction = (jsondata[create_index].car_state == 1)? 0 : 1;
		for(i=0; i<jsondata[create_index].count ; i++)
			southCars[i+southCount] = new Car(0,direction,i, distanceCar);
		southCount = i+southCount;
	}
	else if(jsondata[create_index].signal_state == 1)
	{
		direction = (jsondata[create_index].car_state == 2)? 0 : 1;
		for(i=0; i<jsondata[create_index].count ; i++)
			westCars[i+westCount] = new Car(1,direction,i, distanceCar);
		westCount = i+westCount;
	}
	else if(jsondata[create_index].signal_state == 2)
	{
		direction = (jsondata[create_index].car_state == 3)? 0 : 1;
		for(i=0; i<jsondata[create_index].count ; i++)
			northCars[i+northCount] = new Car(2,direction,i, distanceCar);
		northCount = i+northCount;
	}
	else if(jsondata[create_index].signal_state == 3)
	{
		direction = (jsondata[create_index].car_state == 0)? 0 : 1;
		for(i=0; i<jsondata[create_index].count ; i++)
			eastCars[i+eastCount] = new Car(3,direction,i, distanceCar);
		eastCount = i+eastCount;
	}
	
}

function Car(_start_state, _direction, _index, _distanceCar)
{
	this.start_state = _start_state;						//state
	this.direction = _direction;							//direction 1 = straight, 0 = left turn
	this.index = _index;
	if(this.direction == 0)
	{
		this.x =  cars_potition[this.start_state][0];
		this.y =  cars_potition[this.start_state][1];
	}
	else
	{
		this.x =  cars_potition[this.start_state][2];
		this.y =  cars_potition[this.start_state][3];
	}
	if(this.start_state == 0) this.y += _index * _distanceCar;
	else if(this.start_state == 1) this.x-= _index * _distanceCar;
	else if(this.start_state ==2) this.y -= _index * _distanceCar;
	else this.x += _index * _distanceCar;
	this.carImg = new Image();
	this.carImg.src = './image/'+ this.start_state + '.png';
	this.deg = 0;
	
	Car.prototype.RotateCar = function() //parabolic path
	{
		if(this.start_state == 0)
		{
			if(this.y < 500 && this.y > 280 && this.x > 200 && this.direction != 1)
			{
				this.x -= ((250 - Math.cos((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.cos(this.deg * (Math.PI / 180.0)) * 250.0));
				this.y += ((250 - Math.sin((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.sin(this.deg * (Math.PI / 180.0)) * 250.0));
				this.deg += 7;
			}
			else if(this.deg != 0 && this.direction != 1)
				this.x -= speed;
			else
				this.y -= speed;
		}
		else if(this.start_state == 1)
		{
			if(this.x < 390 && this.x > 200 && this.y > 200 && this.direction != 1)
			{
				this.x -= ((250 - Math.sin((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.sin(this.deg * (Math.PI / 180.0)) * 250.0));
				this.y -= ((250 - Math.cos((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.cos(this.deg * (Math.PI / 180.0)) * 250.0));
				this.deg += 7;
			}
			else if(this.deg != 0 && this.direction != 1)
				this.y -= speed;
			else
				this.x += speed;
		}	
		else if(this.start_state == 2)
		{
			if(this.y < 400 && this.y > 210 && this.x < 460 && this.direction != 1)
			{
				this.x += ((250 - Math.cos((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.cos(this.deg * (Math.PI / 180.0)) * 250.0));
				this.y -= ((250 - Math.sin((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.sin(this.deg * (Math.PI / 180.0)) * 250.0));
				this.deg += 7;
			}
			else if(this.deg != 0 && this.direction != 1)
				this.x += speed;
			else
				this.y += speed;
		}
		else
		{
			if(this.x < 460 && this.x > 230 && this.y < 460 && this.direction != 1)
			{
				this.x += ((250 - Math.sin((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.sin(this.deg * (Math.PI / 180.0)) * 250.0));
				this.y += ((250 - Math.cos((this.deg + 5) * (Math.PI / 180.0)) * 250.0) - (250 - Math.cos(this.deg * (Math.PI / 180.0)) * 250.0));
				this.deg += 7;
			}
			else if(this.deg != 0 && this.direction != 1)
				this.y += speed;
			else
				this.x -= speed;
		}
	}
	Car.prototype.carDraw = function() 
	{
		if((jsondata[current_index*2].signal_state == this.start_state)|| !this.waitCar())
		{
				ctx.save();
				this.RotateCar();
				if(this.deg !=0)	//car image rotate
				{
					ctx.translate(this.x,this.y);
					ctx.rotate((Math.PI/180*-1) * (this.deg));
					ctx.drawImage(this.carImg,0,0);
				}
				else
					ctx.drawImage(this.carImg,this.x,this.y);
				ctx.restore();
	
		}
		else
			ctx.drawImage(this.carImg,this.x,this.y);
	}
	Car.prototype.waitCar = function()
	{
		if(waitflag == 1)
		{
			if(this.start_state+1 == jsondata[current_index*2].signal_state || (this.start_state == 3 && jsondata[current_index*2].signal_state == 0))
				return false;
			if(current_index == 0)
				return false;
		}
		return true;
	}
}

function drawScreen()
{
	ctx.clearRect(0,0,700,700);			//clear Canvas
	waitFlagCal();
	carDeleteCheck();
	ctx.drawImage(back_image, 0, 0);
	drawcars();	
	drawcrosslight();
}

function transSignal()
	{
		if(jsondata[current_index*2].signal_state == 0)
		{
			CrossLightObj[0].CrossLightSignalLeft.src = 'image/signal_green_0.png';
			CrossLightObj[0].CrossLightSignalRight.src = 'image/signal_red_2.png';
			CrossLightObj[1].CrossLightSignalLeft.src = 'image/signal_red_1.png';
			CrossLightObj[1].CrossLightSignalRight.src = 'image/signal_red_3.png';
			CrossLightObj[2].CrossLightSignalLeft.src = 'image/signal_red_2.png';
			CrossLightObj[2].CrossLightSignalRight.src = 'image/signal_green_0.png';
			CrossLightObj[3].CrossLightSignalLeft.src = 'image/signal_red_3.png';
			CrossLightObj[3].CrossLightSignalRight.src = 'image/signal_red_1.png';
			
			CrossLightObj[0].setSignalRight(436,0,0,0);
			CrossLightObj[1].setSignalRight(0,0,0,435);
			CrossLightObj[2].setSignalRight(0,0,282,0);
			CrossLightObj[3].setSignalRight(0,282,0,0);	
		}
		else if(jsondata[current_index*2].signal_state == 1)
		{
			CrossLightObj[0].CrossLightSignalLeft.src = 'image/signal_red_0.png';
			CrossLightObj[1].CrossLightSignalLeft.src = 'image/signal_green_1.png';
			CrossLightObj[2].CrossLightSignalRight.src = 'image/signal_red_0.png';
			CrossLightObj[3].CrossLightSignalRight.src = 'image/signal_green_1.png';
			
			CrossLightObj[0].setSignalRight(360,0,0,0);
			CrossLightObj[1].setSignalRight(0,440,0,0);
			CrossLightObj[2].setSignalRight(0,0,205,0);
			CrossLightObj[3].setSignalRight(0,0,0,280);
		}
		else if(jsondata[current_index*2].signal_state == 2)
		{
			CrossLightObj[0].CrossLightSignalRight.src = 'image/signal_green_2.png';
			CrossLightObj[1].CrossLightSignalLeft.src = 'image/signal_red_1.png';
			CrossLightObj[2].CrossLightSignalLeft.src = 'image/signal_green_2.png';
			CrossLightObj[3].CrossLightSignalRight.src = 'image/signal_red_1.png';
			
			CrossLightObj[0].setSignalRight(0,0,360,0);
			CrossLightObj[1].setSignalRight(0,360,0,0);
			CrossLightObj[2].setSignalRight(205,0,0,0);
			CrossLightObj[3].setSignalRight(0,0,0,205);
		}
		else if(jsondata[current_index*2].signal_state == 3)
		{
			CrossLightObj[0].CrossLightSignalRight.src = 'image/signal_red_2.png';
			CrossLightObj[1].CrossLightSignalRight.src = 'image/signal_green_3.png';
			CrossLightObj[2].CrossLightSignalLeft.src = 'image/signal_red_2.png';
			CrossLightObj[3].CrossLightSignalLeft.src = 'image/signal_green_3.png';
			
			CrossLightObj[0].setSignalRight(0,0,435,0);
			CrossLightObj[1].setSignalRight(0,0,0,360);
			CrossLightObj[2].setSignalRight(285,0,0,0);
			CrossLightObj[3].setSignalRight(0,200,0,0);
		}
	}

function setyellow()
{
	if(jsondata[current_index*2].signal_state == 0)
		{
			CrossLightObj[0].CrossLightSignalLeft.src = 'image/signal_orange_0.png';
			CrossLightObj[2].CrossLightSignalRight.src = 'image/signal_orange_0.png';
			CrossLightObj[0].setSignalRight(400,0,0,0);
			CrossLightObj[2].setSignalRight(0,0,245,0);
		}
		else if(jsondata[current_index*2].signal_state == 1)
		{
			CrossLightObj[1].CrossLightSignalLeft.src = 'image/signal_orange_1.png';
			CrossLightObj[3].CrossLightSignalRight.src = 'image/signal_orange_1.png';
			CrossLightObj[1].setSignalRight(0,400,0,0);
			CrossLightObj[3].setSignalRight(0,0,0,240);
		}
		else if(jsondata[current_index*2].signal_state == 2)
		{
			CrossLightObj[0].CrossLightSignalRight.src = 'image/signal_orange_2.png';
			CrossLightObj[2].CrossLightSignalLeft.src = 'image/signal_orange_2.png';
			CrossLightObj[0].setSignalRight(0,0,400,0);
			CrossLightObj[2].setSignalRight(240,0,0,0);
		}
		else if(jsondata[current_index*2].signal_state == 3)
		{	
			CrossLightObj[1].CrossLightSignalRight.src = 'image/signal_orange_3.png';
			CrossLightObj[3].CrossLightSignalLeft.src = 'image/signal_orange_3.png';
			CrossLightObj[1].setSignalRight(0,0,0,400);
			CrossLightObj[3].setSignalRight(0,240,0,0);
		}
		
	setTimeout(changeState, 5000);
}

function drawcars()
{	
	for(var i=0; i<eastCount ; i++)
		if(eastCars[i] != 0)
			eastCars[i].carDraw();	
	
	for(var i=0; i<southCount ; i++)
		if(southCars[i] != 0)
			southCars[i].carDraw();	
	
	for(var i=0; i<westCount ; i++)
		if(westCars[i] != 0)
			westCars[i].carDraw();	
			
	for(var i=0; i<northCount ; i++)
		if(northCars[i] != 0)
			northCars[i].carDraw();	
}

function waitFlagCal()
{
	if(eastCars[0].x >600) waitflag = 1;
	else if(southCars[0].y >600) waitflag = 1;
	else if(westCars[0].x < 60) waitflag = 1;
	else if(northCars[0].y<60) waitflag = 1;	
	else waitflag = 0;
}

function drawcrosslight()
{
	for(var i=0; i<4; i++)
		CrossLightObj[i].drawCrossLight();
}

function changeState()
{
	var NextStateTime = 0;
	
	if(current_index == 0 && setinter == 0)
	{
		if(jsondata[4*current_index]== undefined)
		{
			alert("Simulation 할 Data 가 부족합니다.")
			return 0;
		}
		
		alert("Start Simulation");
		for(var i=0; i<4 ; i++)
		{
			CreateCars(i*2,jsondata[2*(i+1)].timestamp - jsondata[2*i].timestamp);
			CreateCars(i*2+1,jsondata[2*(i+1)].timestamp - jsondata[2*i].timestamp);
		}
	}
	else
	{
		clearInterval(setinter);
		current_index++;
		
		if(jsondata[(current_index+3)*2] != undefined)
		{
			if(jsondata[(current_index+4)*2] != undefined)
			{
				CreateCars((current_index+3)*2,jsondata[(current_index+4)*2].timestamp - jsondata[(current_index+3)*2].timestamp);
				CreateCars((current_index+3)*2+1,jsondata[(current_index+4)*2].timestamp - jsondata[(current_index+3)*2].timestamp);
				waitflag = 1;
			}
		}

	}

	transSignal();
	if(jsondata[2*(current_index+1)] != undefined)
		NextStateTime = jsondata[2*(current_index+1)].timestamp - jsondata[2*current_index].timestamp;
	else
	{
		alert("Simulation End");	
	}
	
	setinter = setInterval(drawScreen, 100);
	setTimeout(setyellow,NextStateTime-5000);
	
}


function carDeleteCheck()
{	
	var passcheck = 0;
	
	if(jsondata[current_index*2].signal_state == 0)
	{
		for(var i=0; i<southCount ; i++)
			if(southCars[i] != 0)
			{
				if((southCars[i].x<0 && southCars[i].direction == 0)||(southCars[i].y< 0 && southCars[i].direction==1))
				{
					delete southCars[i];
					southCars[i]=0;
					
					if(i == southCount-1 && passcheck== 1) southCount = 0;
				}
				passcheck++;
			}
	}
	else if(jsondata[current_index*2].signal_state == 1)
	{
		for(var i=0; i<westCount ; i++)
			if(westCars[i] != 0)
			{
				if((westCars[i].y<0 && westCars[i].direction == 0  )||(westCars[i].x>700 && westCars[i].direction==1))
				{
					delete westCars[i];
					westCars[i]=0;
					
					if(i == westCount-1 && passcheck== 1) westCount = 0;
				}
				passcheck++;
			}
	}
	else if(jsondata[current_index*2].signal_state == 2)
	{
		for(var i=0; i<northCount ; i++)
			if(northCars[i] != 0)
			{
				if((northCars[i].x>700 && northCars[i].direction == 0 )||(northCars[i].y>700 && northCars[i].direction==1))
				{
					delete northCars[i];
					northCars[i]=0;
					
					if(i == northCount-1 && passcheck== 1) northCount = 0;
				}
				passcheck++;
			}
	}
	else if(jsondata[current_index*2].signal_state == 3)
	{
		for(var i=0; i<eastCount ; i++)
			if(eastCars[i] != 0)
			{
				if((eastCars[i].y>700 && eastCars[i].direction == 0)||(eastCars[i].x<0 && eastCars[i].direction==1))
				{
					delete eastCars[i];
					eastCars[i]=0;	
					
					if(i == eastCount-1 && passcheck== 1) eastCount = 0;
				}
				passcheck++;
			}
	}

}
