/*
*   Autore: Com. Francesco Cesare Girogio Teseo Bosatra
*   
*   Copyright 2023
*
*
*/


#include <stdio.h>
#include <math.h>
#include "grader.h"

int counter;
int discesa;
int atterrato;
float quotaVecchia;

typedef enum
{
    STATE_INIT,
    STATE_FLIGHT,
    STATE_DESCENT,
    STATE_LANDED
} state_t;

state_t stato;

/*
    Queste costanti rappresentano l' errore della strumentazione ottenuta mediante l' analisi dei primi 1600 record del CSV
*/
const float DELTAACCELRAZIONE = 0.162579906f;
const float DELTAROTAZIONI = 0.05802066f;
const float DELTAALTITUDINE = 34.7422f;


void init() { 
    printf("Init!\n"); 
    counter = 0;
    discesa = 0;
    quotaVecchia = 0.0f;
    state_t state = STATE_INIT;
    atterrato = 0;
}

void update(float acc_x, float acc_y, float acc_z, float gyro_x, float gyro_y,
            float gyro_z, float baro)
{
    printf("Update: %f,%f,%f,%f,%f,%f,%f\n", acc_x, acc_y, acc_z, gyro_x,
           gyro_y, gyro_z, baro);

    /*
        Calcolo le norme dei vettori accelerazione e Rotazione
    */       

    float norma_accelerazione = sqrt(pow(acc_x, 2) + pow(acc_y, 2) + pow(acc_z, 2));
    float norma_rotazioni = sqrt(pow(gyro_x, 2) + pow(gyro_y, 2) + pow(gyro_z, 2));
    float differenza_quota = quotaVecchia - baro;


    switch (stato)
    {
    case STATE_INIT:
        /*
            Verifico al fine di determinare un vero lancio che la norma dell' accelerazione sia superiore a g +  3 volte l' errore
            in modo da evitare eventuali falsi decolli
        */
        if (norma_accelerazione > (9.8f + (3.0f* DELTAACCELRAZIONE)))
        {
            liftoff();
            stato = STATE_FLIGHT;
        }
        break;

    case STATE_FLIGHT:
        /* 
         per determinare l' apogeo ho deciso di monitorare per 5 cicli consecutivi la condizione di discesa. se nel corso dei
         cicli di controllo la quaota dovesse salire, il conteggio si resetta. Questo per evitare una potenziale falsa
         lettura della strumentazione di bordo dovuta da variazioni di pressione sulle prese statiche o appunto di precisione 
         del sensore
        */

        if (differenza_quota > 0.0f){
            discesa = 0;
        } else{

            discesa ++;
        }

        if (discesa == 5)
        {
            apogee();
            stato = STATE_DESCENT;
        }
        break;
    case STATE_DESCENT:
    /* 
        Per determinare la condizione di atterraggio certo ho deciso di monitorare per 5 cicli consecutivi 
        le seguenti variabili, tutte calcolate al netto dell' errore massimo risocntrato in fase di analisi del csv:
        1) norma dell' accelerazione == g
        2) norma del vettore rotazione == 0 => Missile fermo
        3) Quota costante
    */
    if ((norma_accelerazione > (9.8f - DELTAACCELRAZIONE)) && 
            (norma_accelerazione < (9.8f + DELTAACCELRAZIONE)) && 
            (norma_rotazioni > (0.0f - DELTAROTAZIONI)) &&
            (norma_rotazioni < (0.0f + DELTAROTAZIONI)) &&
            (differenza_quota > (0.0f - DELTAALTITUDINE)) &&
            (differenza_quota < (0.0f + DELTAALTITUDINE)))
        {
           atterrato ++;
        } else{

           atterrato = 0;
        }


        if (atterrato == 5)
        {
            landed();
            stato = STATE_LANDED;
        }
        break;

    default:
        break;
    }
    
    quotaVecchia = baro;
    counter++;
  
}
