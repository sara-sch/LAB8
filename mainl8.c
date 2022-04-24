/* 
 * File:   mainl8.c
 * Author: saras
 *
 * Created on April 18, 2022, 2:29 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <pic16f887.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000
#define _tmr0_value 236 // 10ms

// Variables
uint8_t banderas;
uint8_t centenas;
uint8_t decenas;
uint8_t unidades;
uint8_t res;
uint8_t tabla[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);
uint8_t udc(uint8_t a); 

// Funciones

uint8_t udc(uint8_t a) {        // Función que brinda las centenas, decenas y unidades
    centenas = a/100;
    res = a%100;
    decenas = res/10;
    unidades = res%10;
}

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){              // Fue interrupción del ADC?
        if(ADCON0bits.CHS == 0){    // Verificamos sea AN0 el canal seleccionado
            PORTC = ADRESH;         // Mostramos ADRESH en PORTC
        }
        else if(ADCON0bits.CHS == 1){
            udc(ADRESH);
        }
        PIR1bits.ADIF = 0;          // Limpiamos bandera de interrupción
    }
    if(INTCONbits.T0IF)         // Mostrar contador en c/d/u con interrupción de TMR0
    {
        PORTB = 0;
        if (banderas == 0b00){
            PORTD = tabla[centenas];
            RB0 = 1;
            banderas = 0b01;    // Cambia al siguiente display
        }
        else if (banderas == 0b01){
            PORTD = tabla[decenas];
            RB1 = 1;
            banderas = 0b10;    // Cambia al siguiente display
        }
        else if (banderas == 0b10){
            PORTD = tabla[unidades];
            RB2 = 1;
            banderas = 0b00;    // Cambia al siguiente display
        }
       INTCONbits.T0IF = 0;
       TMR0 = _tmr0_value;
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            if(ADCON0bits.CHS == 0){
                ADCON0bits.CHS = 1;
            }
            else if(ADCON0bits.CHS == 1){
                ADCON0bits.CHS = 0;
            }
            __delay_us(40);
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversión
        } 
    }
    return;
}

void setup(void){
    ANSEL = 0b00000011; // AN0 como entrada analógica
    ANSELH = 0;         // I/O digitales)
    
    TRISA = 0b00000011; // AN0 como entrada
    PORTA = 0; 
    
    TRISC = 0;
    PORTC = 0;
    
    TRISD = 0;
    PORTD = 0;
    
    TRISB = 0b11111000;         // PORTD0, PORTD1 y PORTD2 como salida
    PORTB = 0;                  // Se limpia PORTD
    
    // Configuración reloj interno
    OSCCONbits.IRCF = 0b0110;   // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuración TMR0
    OPTION_REGbits.T0CS = 0;    // TMR0 con internal clock
    OPTION_REGbits.PSA = 0;     // Prescaler a TMR0
    OPTION_REGbits.PS = 0b111; // PSA 256
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Seleccionamos el AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(40);             // Sample time
    
    // Configuracion interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
    INTCONbits.T0IE = 0;        // Se habilita interrupción en TMR0
    INTCONbits.T0IF = 0;        // Se limpia bandera de interrupción del TMR0

}
