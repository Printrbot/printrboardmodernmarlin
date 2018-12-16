/*
  u8g_com_stm32duino_hw_spi.cpp

  communication interface for SPI protocol
*/

#if defined(STM32F1) || defined(STM32F1xx) || defined(STM32F4) || defined(STM32F4xx)

#include "u8g.h"
#include "SPI.h"

static SPISettings spiConfig;
static uint8_t msgInitCount = 2; // Ignore all messages until 2nd U8G_COM_MSG_INIT


uint8_t u8g_com_stm32duino_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  if (msgInitCount) {
    if (msg == U8G_COM_MSG_INIT) msgInitCount --;
    if (msgInitCount) return -1;
  }

  switch(msg)
  {
    case U8G_COM_MSG_STOP:
      break;
    case U8G_COM_MSG_INIT:
      u8g_SetPIOutput(u8g, U8G_PI_CS);
      u8g_SetPIOutput(u8g, U8G_PI_A0);
      u8g_SetPIOutput(u8g, U8G_PI_RESET);

      u8g_SetPILevel(u8g, U8G_PI_CS, 1);
      
      spiConfig = SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0);
      SPI.begin();
      break;

    case U8G_COM_MSG_ADDRESS:           /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
      u8g_SetPILevel(u8g, U8G_PI_A0, arg_val);
      break;

    case U8G_COM_MSG_CHIP_SELECT:       /* arg_val == 0 means HIGH level of U8G_PI_CS */
      if ( arg_val == 0 ) {
        /* disable */
        u8g_SetPILevel(u8g, U8G_PI_CS, 1);
      } else {
        /* enable */
        u8g_SetPILevel(u8g, U8G_PI_CS, 0); /* CS = 0 (low active) */
      }
      break;

    case U8G_COM_MSG_RESET:
      u8g_SetPILevel(u8g, U8G_PI_RESET, arg_val);
      break;

    case U8G_COM_MSG_WRITE_BYTE:
      SPI.beginTransaction(spiConfig);
      SPI.send(arg_val);
      SPI.endTransaction();
      break;

    case U8G_COM_MSG_WRITE_SEQ:
      SPI.beginTransaction(spiConfig);
      SPI.send((uint8_t *)arg_ptr, arg_val);
      SPI.endTransaction();
      break;
  }
  return 1;
}

#endif // defined(STM32F1) || defined(STM32F1xx) || defined(STM32F4) || defined(STM32F4xx)
