// Load Wi-Fi library
#include <WiFi.h>

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

// Replace with your network credentials
const char* ssid = "YOURSSID";
const char* password = "YOURPASS";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String estado_boton = "off";
String output27State = "off";
int valor_iluminacion = 0;
unsigned long tiempo_apagado;
String segundos, minutos, horas, dias, hall_sensor, temp, luz, accion,ip, porcentaje_luz;
String tiempo_restante;
bool temporizador = false, confirmar_sleep = false;
// Assign output variables to GPIO pins
const int salida_rele = 26;
const int sensor_luz = 35;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(salida_rele, OUTPUT);
  pinMode(sensor_luz, INPUT);

  // Set outputs to LOW
  digitalWrite(salida_rele, HIGH);


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ip = WiFi.localIP().toString();
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  //Vemos si tenemos que apagar la luz
  if((tiempo_apagado < millis()) && temporizador){
      Serial.println("Se apagan los LEDS"); 
      digitalWrite(salida_rele, HIGH);
      temporizador = false;
      accion = " Ninguna";
      Serial.println("Se han apagado los LEDS");
      estado_boton = "off";
      
  }

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();

      
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /boton/on") >= 0) {
              Serial.println("Boton on");
              estado_boton = "on";
              accion = " Encender";
              digitalWrite(salida_rele, LOW);
            } else if (header.indexOf("GET /boton/off") >= 0) {
              Serial.println("Boton off");
              estado_boton = "off";
              temporizador = false;
              accion = " Ninguna";
              accion = " Apagar";
              digitalWrite(salida_rele, HIGH);
            }

            //Tiempo de apagado en 5 segundos
            if (header.indexOf("GET /tiempo/05") >= 0) {
              tiempo_apagado = millis() + 5*1000;
              temporizador = true;
              accion = "Apagar en 5 segundos";
            }

            if (header.indexOf("GET /tiempo/10") >= 0) {
              tiempo_apagado = millis() + 10*1000*60;
              temporizador = true;
              accion = "Apagar en 10 minutos";
            }

            if (header.indexOf("GET /tiempo/30") >= 0) {
              tiempo_apagado = millis() + 30*1000*60;
              temporizador = true;
              accion = "Apagar en 30 minutos";
            }

            if (header.indexOf("GET /tiempo/60") >= 0) {
              tiempo_apagado = millis() + 60*1000*60;
              temporizador = true;
              accion = "Apagar en 1h";
            }

            if (header.indexOf("GET /tiempo/90") >= 0) {
              tiempo_apagado = millis() + 90*1000*60;
              temporizador = true;
              accion = "Apagar en 1:30h";
            }

            if (header.indexOf("GET /tiempo/120") >= 0) {
              tiempo_apagado = millis() + 120*1000*60;
              temporizador = true;
              accion = "Apagar en 2h";
            }

            if (header.indexOf("GET /tiempo/180") >= 0) {
              tiempo_apagado = millis() + 180*1000*60;
              temporizador = true;
              accion = "Apagar en 3h";
            }

            if (header.indexOf("GET /tiempo/240") >= 0) {
              tiempo_apagado = millis() + 240*1000*60;
              temporizador = true;
              accion = "Apagar en 4h";
            }
            if (header.indexOf("GET /tiempo/eliminar") >= 0) {
              tiempo_apagado = 0;
              temporizador = false;
              accion = " Eliminar temportizador y apagar";
              digitalWrite(salida_rele, HIGH);
              estado_boton = "off";
            }

            if(temporizador){
              //Encendemos los leds
              digitalWrite(salida_rele, LOW);
              estado_boton = "on";
            }


            if (header.indexOf("GET /sleep/confirmar") >= 0) {
              if(confirmar_sleep){

                confirmar_sleep = false;
                esp_light_sleep_start();
              }
            }
            else{
              confirmar_sleep = false;
            }


            //Sleep mode
            if (header.indexOf("GET /sleep") >= 0) {
              confirmar_sleep = true;
            }
            
            
            
          
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 12px 30px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".recargar { background-color: #5fccf9; border: none; color: black; padding: 12px 30px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".eliminar { background-color: #ff1010; border: none; color: black; padding: 12px 30px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".peligro { background-color: #fec52a; border: none; color: black; padding: 12px 30px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>EL PC DE LOS PECES</h1>");
            
            String coso = "/";
            client.println("<a href=" +coso+"><button class=\"recargar\">Refrescar</button></a></p>");
            
            client.println("<hr> "); 
            
            client.println("<p><h4>Estado de las luces: " + estado_boton + "</h4></p>");
            valor_iluminacion = analogRead(sensor_luz);
            porcentaje_luz = map(valor_iluminacion, 500, 4200, 0, 100);
            client.println("<p><h4>Luz ambiental: " + porcentaje_luz + "%</h4></p>");
            // If the estado_boton is off, it displays the ON button       
            if (estado_boton=="off") {
              client.println("<p><a href=\"/boton/on\"><button class=\"button\">ENCENDER</button></a>");
            } else {
              client.println("<p><a href=\"/boton/off\"><button class=\"button button2\">APAGAR</button></a>");
            } 
            




            client.println("<hr> ");
            client.println("<p><h4>Temporizadores: </h4></p>");
            if(temporizador){
              tiempo_restante = (tiempo_apagado - millis()) /1000/60 ;
              client.println("<p>Tiempo restante: "+ tiempo_restante + "minutos</p>");
            }

            client.println("<a href=\"/tiempo/05\"><button class=\"button\">5 segundos</button></a>");
            client.println("<a href=\"/tiempo/10\"><button class=\"button\">10 minutos</button></a>");
            client.println("<p></p>");
            client.println("<a href=\"/tiempo/30\"><button class=\"button\">30 minutos</button></a>");
            client.println("<a href=\"/tiempo/60\"><button class=\"button\">1 hora</button></a>");
            client.println("<p></p>");
            client.println("<a href=\"/tiempo/90\"><button class=\"button\">1:30 horas</button></a>");
            client.println("<a href=\"/tiempo/120\"><button class=\"button\">2 horas</button></a>");
            client.println("<p></p>");
            client.println("<a href=\"/tiempo/180\"><button class=\"button\">3 horas</button></a>");
            client.println("<a href=\"/tiempo/240\"><button class=\"button\">4 horas</button></a>");
            client.println("<p></p>");
            client.println("<p><a href=\"/tiempo/eliminar\"><button class=\"eliminar\">Eliminar temporizador</button></a></p>");
            


            
            client.println("<hr> ");
            client.println("<p><h4>Estadisticas ESP32:</h4></p>");
            segundos = (millis()/1000)%60;
            minutos = (millis()/1000)/60;
            horas = (millis()/1000)/3600;
            dias = (millis()/1000)/86400;
            hall_sensor = hallRead();
            temp = (temprature_sens_read() - 32) / 1.8;

            client.println("<p>Tiempo en funcionamiento: "+dias+"d "+horas+"h "+minutos+"m "+segundos+"s" + "</p>");
            client.println("<p>Sensor Hall: "+ hall_sensor + "</p>");
            if(temp == "53.33"){
              client.println("<p>Temperatura: Sensor no presente</p>");
            }else{
              client.println("<p>Temperatura: "+temp+" C</p>");
            }

            luz = analogRead(sensor_luz);

            client.println("<p>Valor fotoresistencia: "+ luz + "</p>");

            client.println("<p>Ultima accion ejecutada: "+ accion + "</p>");

            

            if(temporizador){
              tiempo_restante = (tiempo_apagado - millis()) /1000/60;
              client.println("<p>Tiempo restante para la accion: "+ tiempo_restante + " minutos </p>");
            }
            
            
           
            //Para entrar en modo sleep
            if(!confirmar_sleep){
              client.println("<a href=\"/sleep\"><button class=\"peligro\">Sleep mode</button></a>");
            }else{
              client.println("<a href=\"/sleep/confirmar\"><button class=\"eliminar\">Confirmar sleep mode</button></a>");
            }
            
            

            client.println("<p></p>");    
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
