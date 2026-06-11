/* boards/34401A.h */


#define LCDPICO_MODEL     "34401A"


#define LED_PIN 25

#define SENSOR_COUNT 1
#define SENSOR_READ_PIN  -1
#define SENSOR_READ_ADC  ADC_TEMPERATURE_CHANNEL_NUM  /* Internal Temperature sensor */


/* Interface Pins */

/* I2C */
#define I2C_HW    1  /* 0=i2c0, 1=i2c1 */
#define SDA_PIN   2
#define SCL_PIN   3

/* Serial */
#define TX_PIN    0
#define RX_PIN    1


/* LCD Panel SPI interface */
#define LCD_SPI_HW     SPI_INSTANCE(0)
#define LCD_CS_PIN     4
#define LCD_CLK_PIN    6
#define LCD_MOSI_PIN   7
#define LCD_MISO_PIN   -1  // 8
#define LCD_RESET_PIN  -1

/* LCD Display Controller SPI interface */
#define LCM_SPI_HW     SPI_INSTANCE(1)
#define LCM_CS_PIN     13
#define LCM_CLK_PIN    14
#define LCM_MOSI_PIN   15
#define LCM_MISO_PIN   12
#define LCM_RESET_PIN  9
#define LCM_INT_PIN    11
#define LCDM_BL_PIN    10


#define SCK_PIN        18
#define DI_PIN         16
#define DO_PIN         4
#define RST_PIN        20
#define INT_PIN        19

#define TTL_SERIAL   1
#define TTL_SERIAL_UART uart0
#define TTL_SERIAL_SPEED 115200
