const http = require("http");
const express = require('express');
const app = express();

const host = 'localhost';
const port = 3000;

const mqtt = require('mqtt')
const client  = mqtt.connect("mqtt://<IP>")
const topic1 = 'esp32/temp'
const topic2 = 'esp32/humd'

var temp;
var humd;

var messages = {
   1:{
      temperature: '',
      humidity: '', 
      author: "Me"}
};


client.on('connect', function () {
    console.log("MQTT connected");
    client.subscribe(topic1);
    client.subscribe(topic2);

    client.on('message', function (topic, message){
            if (topic === topic1){
               temp = message.toString();
               messages[1].temperature = temp;
               console.log(temp)
            }
            if (topic === topic2){
               humd = message.toString();
               messages[1].humidity = humd;
               console.log(humd)   
        }
        
    })
    
});


app.get('/messages/:id',(req,res)=>{
  res.send(messages);
 }
);

//const server = http.createServer(requestListener);
app.listen(port, host, () => {
  console.log(`Server is running on http://${host}:${port}`);
});
