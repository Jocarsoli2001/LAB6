/**
 * Laboratorio 6
 *
 * José Santizo (20185)
 *
 * Electrónica digital 2
 *
 */

//--------------------------------- Incluir librerías ----------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"


//------------------------------ Definición de constantes ------------------------------
#define XTAL 16000000

//-------------------------------- Varibles de programa --------------------------------
int Valor_boton = 0;
int i = 0;
uint8_t Encendido_R = 0;
uint8_t Encendido_G = 0;
uint8_t Encendido_B = 0;
uint8_t Datos_UART = 0;
uint8_t Color_LED = 2;
uint32_t Toggle = 0;
uint32_t timer0Load;

//------------------------------- Prototipo de funciones -------------------------------
void delay(uint32_t msec);
void setup(void);
void setup_timer0(void);
void Timer0IntHandler(void);
void UART0IntHandler(void);
void setup_UART0(void);


//--------------------------------------- Main -----------------------------------------
int main(void)
{
    // ---------------------------------------------------------------------------------
    // Setup
    // ---------------------------------------------------------------------------------
    setup();

    // ---------------------------------------------------------------------------------
    // Loop principal
    // ---------------------------------------------------------------------------------

	while(1){
	}
}

//-------------------------------------- Subrutinas ------------------------------------
//**************************************************************************************************************
// Función de delay en milisegundos
//**************************************************************************************************************
void delay(uint32_t msec)
{
    for (i = 0; i < msec; i++)
    {
        SysCtlDelay(10000);                                                                         // 1 ms de delay
    }

}

//**************************************************************************************************************
// Función de setup para todo el sistema
//**************************************************************************************************************
void setup(void)
{
    // Asignación del reloj del sistema
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);               // 40 MHz

    // Habilitación del reloj para puertos
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                                                    // Puerto F

    IntMasterEnable();                                                                              // Se habilitan las interrupciones Globales

    // Configuración del timer 0
    setup_timer0();

    // Configuración de UART 0
    setup_UART0();

    // Pines como entradas y salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);                       // Pines 1, 2 y 3 del puerto F como salida
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    // Configuración de los pines como input
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);        // Pin 4 como entrada con weak pull up


}

//**************************************************************************************************************
// Función para setear los timers
//**************************************************************************************************************
void setup_timer0(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);                                                   // Habilitar el periférico del timer 0

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){                                            // Esperar a que el módulo de timer 0 se inicie
    }

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);                                                // Configurar el timer 0 como periódico y completo de 32 bits

    timer0Load = (SysCtlClockGet())/2;
    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, timer0Load - 1);                                          // Ingresar valor de timer 0 para generar un desborde a 1 segundo

    IntEnable(INT_TIMER0A);                                                                         // Habilitar interrupciones del timer 0

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                                // Se habilita interrupción por Timeout

    TimerEnable(TIMER0_BASE, TIMER_BOTH);                                                           // Se habilita el Timer
}

//**************************************************************************************************************
// Función para setear UART
//**************************************************************************************************************
void setup_UART0(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);                                                    // Se habilita el puerto A, donde se encuentra RX y TX de UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                                                    // Habilitar el reloj para el UART0

    GPIOPinConfigure(GPIO_PA0_U0RX);                                                                // Habilitar pines 0 y 1 del puerto A para poder utilizarlos como RX y TX
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);                                      // Pines de UART

    /* Setear configuración de UART. */
    UARTConfigSetExpClk(
            UART0_BASE, SysCtlClockGet(), 115200,
            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);                      // Configurar UART 0 con baud rate de 115200, datos de 8 bits, 1 bit de stop y sin paridad

    IntEnable(INT_UART0);                                                                           // Habilitar la interrupción de UART0
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART0_BASE,UART0IntHandler);                                                    // Asigna interrupción al handler de UART0
    UARTEnable(UART0_BASE);
}

//**************************************************************************************************************
// Handler de interrupción de timer 0
//**************************************************************************************************************
void Timer0IntHandler(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                                 // Reset de la interrupción

    if(Toggle == 0){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);                     // Alternar la LED RGB de la TIVA C entre encendido y apagado en cierto color, utilizando la bandera "Toggle"
        Toggle = 1;
    }
    else if(Toggle == 1){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, Color_LED);
        Toggle = 0;
    }
}

//**************************************************************************************************************
// Handler de interrupción de UART 0
//**************************************************************************************************************
void UART0IntHandler(void){

    UARTIntClear(UART0_BASE, UART_INT_RX| UART_INT_RT);                                             // Reiniciar la bandera de interrupción de comunicación UART

    Datos_UART =  UARTCharGetNonBlocking(UART0_BASE);                                               // Leer el pin RX de la TIVA C para guardar los valores del puerto Serial en "Datos_UART"

    /*
     * NOTA: Si alguna de las bandera de "Encendido" se encuentra en 0, quiere decir que el toggle se apagó y la luz LED RGB debería estár apagada
     */

    if(Datos_UART == 'R' && Encendido_R == 0){                                                      // Para poder hacer que se prenda o se apague el color de LED indicado por puerto serial, se utilizan banderas
        Color_LED = 2;
        Encendido_R = 1;
    }
    else if(Datos_UART == 'R' && Encendido_R == 1){                                                 // Si la bandera "Encendido_R" está en 1 quiere decir que la luz LED roja está haciendo el toggle
        Color_LED = 0;
        Encendido_R = 0;
    }
    else if(Datos_UART == 'G' && Encendido_G == 0){
        Color_LED = 8;
        Encendido_G = 1;
    }
    else if(Datos_UART == 'G' && Encendido_G == 1){                                                 // Si la bandera "Encendido_G" está en 1, quiere decir que la luz LED verde está haciendo el toggle
        Color_LED = 0;
        Encendido_G = 0;
    }
    else if(Datos_UART == 'B' && Encendido_B == 0){
        Color_LED = 4;
        Encendido_B = 1;
    }
    else if(Datos_UART == 'B' && Encendido_B == 1){                                                 // Si la bandera "Encendido_B" está en 1, quiere decir que la luz LED azul está haciendo el toggle
        Color_LED = 0;
        Encendido_B = 0;
    }
}

