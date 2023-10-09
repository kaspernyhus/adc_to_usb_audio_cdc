#pragma once
#include "hal/gpio_types.h"

/*
    Pin mapping for Classics Adapter
*/

/* LED Pin */
#define LED_GPIO        GPIO_NUM_15

/* Power ADC MUX Signals */
#define PWR_ADC_MUX_OUT GPIO_NUM_4      /* Analog input ADC1_CH3*/
#define PWR_ADC_MUX_S0  GPIO_NUM_7
#define PWR_ADC_MUX_S1  GPIO_NUM_6

/* I2S Signals */
#define I2S_BCLK_IO     GPIO_NUM_48
#define I2S_LRCLK_IO    GPIO_NUM_47
#define I2S_ADC_DATA_IO GPIO_NUM_45

/* MasterLink Control */
#define ML_UART_RX      GPIO_NUM_38
#define ML_UART_TX      GPIO_NUM_37

/* DataLink and Powerlink */
#define DL_PL_UART_RX   GPIO_NUM_18
#define DL_PL_UART_TX   GPIO_NUM_17
#define PL_SENSE        GPIO_NUM_21

/* User Button */
#define USER_BUTTON     GPIO_NUM_8

/* Boot select pins */
#define LATCH_RESET     GPIO_NUM_9
#define LATCH_SET       GPIO_NUM_10

/* Connector controls */
#define ADC_SEL         GPIO_NUM_12
#define GAIN_SEL        GPIO_NUM_13
#define CONN_EN         GPIO_NUM_35
#define ML_PL_SEL       GPIO_NUM_36


/* Expanison Board Pins */
#define EXP_IO01        GPIO_NUM_1     // Header pin 1
#define EXP_IO02        GPIO_NUM_2     // Header pin 2
#define EXP_IO14        GPIO_NUM_14    // Header pin 3
#define EXP_IO11        GPIO_NUM_11    // Header pin 4
#define EXP_IO46        GPIO_NUM_46    // Header pin 5
#define EXP_IO03        GPIO_NUM_3     // Header pin 6
#define EXP_IO05        GPIO_NUM_5     // Header pin 10
#define EXP_IO16        GPIO_NUM_16    // Header pin 13

/* JTAG */
#define EXP_IO39        GPIO_NUM_39    /* JTAG MTCK */
#define EXP_IO40        GPIO_NUM_40    /* JTAG MTDO */
#define EXP_IO41        GPIO_NUM_41    /* JTAG MTDI */
#define EXP_IO42        GPIO_NUM_42    /* JTAG MTMS */