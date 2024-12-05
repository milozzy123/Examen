#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"

// MQTT configuratie
#define ADDRESS     "tcp://192.168.0.149:1883"
#define CLIENTID    "RaspberryPiClient"
#define TOPIC       "Yoepie"
#define QOS         1

// Functie om berichten naar een tekstbestand te schrijven
void writeMessageToFile(const char *Message, int MessageLength) {
    FILE *file = fopen("helloworld.txt", "a"); // Open bestand in append-modus
    if (file == NULL) {
        printf("Fout bij openen van bestand.\n");
        return;
    }

    fprintf(file, "%.*s\n", MessageLength, Message); // Alleen het bericht opslaan
    fclose(file);
    printf("Bericht opgeslagen in helloworld.txt\n");
}

// Callback-functie voor ontvangen berichten
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Ontvangen bericht: %.*s\n", message->payloadlen, (char *)message->payload);

    // Schrijf het bericht naar een bestand
    writeMessageToFile((char *)message->payload, message->payloadlen);

    // Vrijgeven van resources
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Functie om verbinding te maken met MQTT, berichten te ontvangen en op te slaan
void receiveAndSaveMQTTMessages() {
    MQTTClient client;
    int rc;

    // Maak de MQTT-client aan
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Stel de callback in
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    // Verbind met de broker
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Fout bij verbinden: %d\n", rc);
        return;
    }
    printf("Verbonden met broker op %s\n", ADDRESS);

    // Abonneer op het onderwerp
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS) {
        printf("Fout bij abonneren op topic: %d\n", rc);
        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
        return;
    }
    printf("Geabonneerd op topic: %s\n", TOPIC);

    // Wachten op berichten (oneindige lus)
    printf("Wachten op berichten...\n");
    while (1) {
        // Berichten worden verwerkt via de callback
    }

    // Afsluiten (niet bereikt in deze implementatie)
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}

int main() {
    receiveAndSaveMQTTMessages();
    return 0;
}
