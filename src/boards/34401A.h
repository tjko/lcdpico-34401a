/* boards/34401A.h */


#define LCDPICO_MODEL     "34401A"


#define LED_PIN 25

#define SENSOR_COUNT 1
#define SENSOR_READ_PIN  -1
#define SENSOR_READ_ADC  ADC_TEMPERATURE_CHANNEL_NUM  /* Internal Temperature sensor */


/* Interface Pins */

/* I2C */
#define I2C_HW    2  /* 1=i2c0, 2=i2c1, 0=bitbang... */
#define SDA_PIN   2
#define SCL_PIN   3

/* Serial */
#define TX_PIN    0
#define RX_PIN    1

/* SPI */
#define SPI_SHARED     1 /* 0 = dedicated pins, 1 = shared with serial/i2c */
#define SCK_PIN        2
#define MOSI_PIN       3
#define MISO_PIN      -1
#define CS_PIN         1

#define DC_PIN         0
#define LCD_RESET_PIN -1
#define LCD_LIGHT_PIN -1


#define TTL_SERIAL   1
#define TTL_SERIAL_UART uart0
#define TTL_SERIAL_SPEED 115200
