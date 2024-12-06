#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "MQTTClient.h"

#define ADDRESS "tcp://192.168.0.105:1883" // !! IP-address:Port-ID
#define QOS 1
#define CLIENTID "Milo"
#define SUB_TOPIC "P1/MD5"
#define TOPIC_LEN 120
#define TIMEOUT 500L
#define FileName "/home/milo/examen/Data.txt"

char datum_tijd_stroom[20];
int tarief_indicator;
float actueel_stroomverbruik;
float actueel_spanning;
float totaal_dagverbruik;
float totaal_nachtverbruik;
float totaal_dagopbrengst;
float totaal_nachtopbrengst;
char datum_tijd_gas[20];
float totaal_gasverbruik;

bool first = true;
int i;
float Totaal_verbruik;
float Totaal_opbrengst;
float gasverbruik;
float verbruik;
float opbrengst;
FILE *fp;
int stop = 0;
char datum[10];
bool print;
float vorigeVerbruik;
float vorigeOpbrengst;
float vorigeGasverbruik;
float momenteel_gasverbruik;

void SchrijfDataInFile(char *bericht)
{
    FILE *fptr;

    // Open a file in append mode
    fptr = fopen("Data.txt", "a");

    // Append some text to the file
    fprintf(fptr, "%s\n", bericht);

    // Close the file
    fclose(fptr);
}
int IsVolgendeDag(char *datum_tijd)
{
    int vorigeDag, vorigeMaand, vorigeJaar;
    char *token3 = strtok(datum_tijd, "-");
    strncpy(datum, token3, sizeof(datum));
    char *token2 = strtok(datum, ".");
    int dag = atoi(token2);
    token2 = strtok(NULL, ".");
    int maand = atoi(token2);
    token2 = strtok(NULL, ".");
    int jaar = atoi(token2);
    if (i == 1)
    {
        vorigeDag = dag;
        vorigeMaand = maand;
        vorigeJaar = jaar;
        print = true;
        i = 0;
    }
    else if (dag > vorigeDag && maand >= vorigeMaand && jaar >= vorigeJaar)
    {
        vorigeDag = dag;
        vorigeMaand = maand;
        vorigeJaar = jaar;
        print = true;
    }
    else
    {
        print = false;
    }
}

void Waardes_Berekenen()
{
    if (first == true)
    {
        vorigeGasverbruik = totaal_gasverbruik * 11.55;
        vorigeOpbrengst = totaal_dagopbrengst + totaal_nachtopbrengst;
        vorigeVerbruik = totaal_dagverbruik + totaal_nachtverbruik;
    }
    gasverbruik = vorigeGasverbruik - (totaal_gasverbruik * 11.55);
    verbruik = vorigeVerbruik - (totaal_dagverbruik + totaal_nachtverbruik);
    opbrengst = vorigeOpbrengst - (totaal_dagopbrengst + totaal_nachtopbrengst);
    
    vorigeGasverbruik = totaal_gasverbruik * 11.55;
    vorigeOpbrengst = totaal_dagopbrengst + totaal_nachtopbrengst;
    vorigeVerbruik = totaal_dagverbruik + totaal_nachtverbruik;
}
// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char *bericht = message->payload;
    SchrijfDataInFile(bericht);
    // print incoming message
    char *JuisteDatumTijd;
    char *token = strtok(bericht, ";");
    strncpy(datum_tijd_stroom, token, sizeof(datum_tijd_stroom));
    JuisteDatumTijd = datum_tijd_stroom;
    token = strtok(NULL, ";");
    tarief_indicator = atof(token);

    token = strtok(NULL, ";");
    actueel_stroomverbruik = atof(token);

    token = strtok(NULL, ";");
    actueel_spanning = atof(token);

    token = strtok(NULL, ";");
    totaal_dagverbruik = atof(token);

    token = strtok(NULL, ";");
    totaal_nachtverbruik = atof(token);

    token = strtok(NULL, ";");
    totaal_dagopbrengst = atof(token);

    token = strtok(NULL, ";");
    totaal_nachtopbrengst = atof(token);

    token = strtok(NULL, ";");
    strncpy(datum_tijd_gas, token, sizeof(datum_tijd_gas));

    token = strtok(NULL, ";");
    totaal_gasverbruik = atof(token);

    if (tarief_indicator != 0)
    {
        if (first == true)
        {
            printf("STARTWAARDEN\n");
            printf("\n");
            printf("DATUM - TIJD:   %s\n", JuisteDatumTijd);
            printf("DAG     Totaal verbruik     = %f    kWh\n", totaal_dagverbruik);
            printf("DAG     Totaal opbrengst    = %f    kWh\n", totaal_dagopbrengst);
            printf("NACHT   Totaal verbruik     = %f    kWh\n", totaal_nachtverbruik);
            printf("NACHT   Totaal opbrengst    = %f    kWh\n", totaal_nachtopbrengst);
            printf("GAS     Totaal verbruik     = %f    m3\n", totaal_gasverbruik);
            printf("-----------------------------------------------------------------\n");
            printf("TOTALEN:\n");
            printf("-----------------------------------------------------------------\n");
            first = false;
            i = 1;
        }
        Waardes_Berekenen();
        IsVolgendeDag(datum_tijd_stroom);
        if (print == true)
        {
            printf("Datum: %s\n", datum_tijd_stroom);
            printf("-------------------\n");
            printf("STROOM: \n");
            printf("            Totaal verbruik     =   %f  kWh\n", verbruik);
            printf("            Totaal opbrengst    =   %f  kWh\n", opbrengst);
            printf("GAS: \n");
            printf("            Totaal verbruik     =   %f  kWh\n", gasverbruik);
            printf("*\n");
            print = false;
        }
    }
    else
    {
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("Einde van dit rapport\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        stop = 1;
    }
    return 1;
}

// This function is called upon when the connection to the mqtt-broker is lost
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main()
{
    fp = fopen(FileName, "w"); // Open de file
    if (fp == NULL)
    {
        printf("Fout: bestand %s kan niet worden geopend.\n", FileName);
        return 0;
    }
    // Open MQTT client for listening
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, client, connlost, msgarrvd, NULL);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n\n", SUB_TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, SUB_TOPIC, QOS);
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("Elektriciteit- en gas verbruik  - totalen per dag\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    // Keep the program running to continue receiving and publishing messages
    while (stop != 1)
    {
        ;
        ;
    }
    fclose(fp);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
