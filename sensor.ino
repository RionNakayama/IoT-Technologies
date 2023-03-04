#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define DHTPIN 22    
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);  //client=PubSubClient


const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

struct node {
  float temp;
  float humd;
  struct node *next;
};

//create a new pointer(named "list") to node structure
struct node *list = NULL;  

void Ave(struct node*list){
  int count = 0;
  float sumT = 0;
  float sumH = 0;
  float aveT = 0;
  float aveH = 0;
  char strT[8];
  char strH[8];

  struct node * calc = list;

  while (calc != NULL){
    count ++;
    sumT += calc->temp;
    sumH += calc->humd;
    calc = calc->next;
  }

  aveT = sumT / count;
  aveH = sumH / count;

  dtostrf(aveT, 1, 2, strT);
  dtostrf(aveH, 1, 2, strH);

  Serial.print("temperature: ");
  Serial.print(strT);
  Serial.print("°C   Humidity: ");
  Serial.print(strH);
  Serial.println("%");
  
  client.publish("esp32/temp", strT);
  client.publish("esp32/humd", strH);
  
}

void insert(struct node ** list, float temp, float humd){
  struct node*new_node = (struct node *) malloc(sizeof(struct node)); 
  struct node * last = *list;

  new_node -> temp = temp;
  new_node -> humd = humd;
  new_node -> next = NULL;

  if (*list == NULL){
    *list = new_node;
    return;
  }
  while (last -> next != NULL){
    last -> next = new_node;
    return;
  }
  
}

void setup() {
  Serial.begin(9600);

  dht.begin();

  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect(){
  while (!client.connected()){
    Serial.print("trying to connect MQTT...");
    if (client.connect("ESP8266Client")){ //?
      Serial.println("connected!");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("  try again");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()){
    reconnect();
  }
  client.loop(); 
  
  delay(2000);
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  insert(&list, temperature, humidity);
  Ave(list);
}
