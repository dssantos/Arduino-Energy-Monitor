// Adaptado de http://labdegaragem.com/forum/topics/comunica-o-serial-do-arduino-com-esp8266-esp-01?page=1&commentId=6223006%3AComment%3A606755&x=1#6223006Comment606755

#include <ESP8266WiFi.h>  //essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include "ACS712.h"

// Mude para o modelo do ACS712 que está usando. Valores possíveis: 5A,20A e 30A
// Sensor ligado na porta analógica A0
ACS712 sensor(ACS712_20A, A0);

float I = 0;
float V = 0;

//defines
#define SSID_REDE     "XXXXXXXXX"  //coloque aqui o nome da rede que se deseja conectar
#define SENHA_REDE    "XXXXXXXXX"  //coloque aqui a senha da rede que se deseja conectar
#define INTERVALO_ENVIO_THINGSPEAK  40000  //intervalo entre envios de dados ao ThingSpeak (em ms)

//constantes e variáveis globais
char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "XXXXXXXXXXX"; //Informe a chave do Thingspeak
long lastConnectionTime; 
WiFiClient client;

//prototypes
void EnviaInformacoesThingspeak(String StringDados);
void FazConexaoWiFi(void);
float FazLeituraUmidade(void);

/*
 * Implementações
 */

//Função: envia informações ao ThingSpeak
//Parâmetros: String com a  informação a ser enviada
//Retorno: nenhum
void EnviaInformacoesThingspeak(String StringDados)
{
    if (client.connect(EnderecoAPIThingSpeak, 80))
    {         
        //faz a requisição HTTP ao ThingSpeak
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+ChaveEscritaThingSpeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(StringDados.length());
        client.print("\n\n");
        client.print(StringDados);
  
        lastConnectionTime = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
     }   
}

//Função: faz a conexão WiFI
//Parâmetros: nenhum
//Retorno: nenhum
void FazConexaoWiFi(void)
{
    client.stop();
    Serial.println("Conectando-se à rede WiFi...");
    Serial.println();  
    delay(1000);
    WiFi.begin(SSID_REDE, SENHA_REDE);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connectado com sucesso!");  
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    delay(1000);
}

void setup()
{  
    Serial.begin(9600);
    lastConnectionTime = 0; 
    FazConexaoWiFi();
    Serial.println("Aguarde. Calibrando..."); 
    sensor.calibrate();
    Serial.println("Fim da calibração");
}

//loop principal
void loop()
{
    char FieldWatts[11];
    
    //Força desconexão ao ThingSpeak (se ainda estiver desconectado)
    if (client.connected())
    {
        client.stop();
        Serial.println("- Desconectado do ThingSpeak");
        Serial.println();
    }
    
    // A frequência da corrente alternada no Brasil é 60 Hz. Mude caso necessário
    // A voltagem, no meu caso é 127
    for(int i=300; i>0; i--){        
      V = 127;
      I += sensor.getCurrentAC(60);
      delay(1);
    }
    I /= 300;
    float P = V * I;
    
    int Watts = (int)P; //trunca umidade como número inteiro
    
    //verifica se está conectado no WiFi e se é o momento de enviar dados ao ThingSpeak
    if(millis() - lastConnectionTime > INTERVALO_ENVIO_THINGSPEAK)
    {
        sprintf(FieldWatts,"field1=%d",Watts);
        EnviaInformacoesThingspeak(FieldWatts);
        Serial.println(String("Corrente = ") + I);
        Serial.println(String("Potência  = ") + P + "\n");
        
    }

     delay(1000);
}
