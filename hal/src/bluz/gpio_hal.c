/**
 ******************************************************************************
 * @file    gpio_hal.c
 * @authors Matthew McGowan
 * @version V1.0.0
 * @date    27-Sept-2014
 * @brief
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "gpio_hal.h"
#include "pinmap_impl.h"
#undef SCK
#undef MOSI
#undef MISO
#undef SS
#include "nrf_gpio.h"
#include "nrf_adc.h"

/* Private typedef ----------------------------------------------------------*/

/* Private define -----------------------------------------------------------*/

/* Private macro ------------------------------------------------------------*/

/* Private variables --------------------------------------------------------*/
PinMode digitalPinModeSaved = PIN_MODE_NONE;

/* Extern variables ---------------------------------------------------------*/

/* Private function prototypes ----------------------------------------------*/
inline bool is_valid_pin(pin_t pin)
{
    return pin<TOTAL_PINS;
}


PinMode HAL_Get_Pin_Mode(pin_t pin)
{
    return (!is_valid_pin(pin)) ? PIN_MODE_NONE : PIN_MAP[pin].pin_mode;
}

PinFunction HAL_Validate_Pin_Function(pin_t pin, PinFunction pinFunction)
{
    if (!is_valid_pin(pin))
        return PF_NONE;
    if (pinFunction==PF_ADC && PIN_MAP[pin].adc!=true)
        return PF_ADC;
    if (pinFunction==PF_TIMER)
        return PF_TIMER;
    return PF_DIO;
}

/*
 * @brief Set the mode of the pin to OUTPUT, INPUT, INPUT_PULLUP,
 * or INPUT_PULLDOWN
 */
void HAL_Pin_Mode(pin_t pin, PinMode setMode)
{
    switch (setMode)
    {
            
        case OUTPUT:
            nrf_gpio_cfg_output(PIN_MAP[pin].gpio_pin);
            PIN_MAP[pin].pin_mode = OUTPUT;
            break;
            
        case INPUT:
            nrf_gpio_cfg_input(PIN_MAP[pin].gpio_pin,
                               NRF_GPIO_PIN_NOPULL);
            PIN_MAP[pin].pin_mode = INPUT;
            break;
            
        case INPUT_PULLUP:
            nrf_gpio_cfg_input(PIN_MAP[pin].gpio_pin,
                                     NRF_GPIO_PIN_PULLUP);
            PIN_MAP[pin].pin_mode = INPUT_PULLUP;
            break;
            
        case INPUT_PULLDOWN:
            nrf_gpio_cfg_input(PIN_MAP[pin].gpio_pin,
                               NRF_GPIO_PIN_PULLDOWN);
            PIN_MAP[pin].pin_mode = INPUT_PULLDOWN;
            break;
            
        case AF_OUTPUT_PUSHPULL:  //Used internally for Alternate Function Output PushPull(TIM, UART, SPI etc)
            break;
            
        case AF_OUTPUT_DRAIN:   //Used internally for Alternate Function Output Drain(I2C etc)
            break;
            
        case AN_INPUT:        //Used internally for ADC Input
            nrf_adc_init(ADC_RES_10bit, PIN_MAP[pin].gpio_pin, ADC_INT_DISABLED);
            PIN_MAP[pin].pin_mode = AN_INPUT;
            break;
            
        default:
            break;
    }
}

/*
 * @brief Saves a pin mode to be recalled later.
 */
void HAL_GPIO_Save_Pin_Mode(PinMode mode)
{
    digitalPinModeSaved = mode;
}

/*
 * @brief Recalls a saved pin mode.
 */
PinMode HAL_GPIO_Recall_Pin_Mode()
{
  return digitalPinModeSaved;
}

/*
 * @brief Sets a GPIO pin to HIGH or LOW.
 */
void HAL_GPIO_Write(uint16_t pin, uint8_t value)
{
    //If the pin is used by analogWrite, we need to change the mode
    if(PIN_MAP[pin].pin_mode != OUTPUT)
    {
        HAL_Pin_Mode(pin, OUTPUT);
    }
    
    if(value == 0)
    {
        nrf_gpio_pin_clear(PIN_MAP[pin].gpio_pin);
    }
    else
    {
        nrf_gpio_pin_set(PIN_MAP[pin].gpio_pin);
    }
}

/*
 * @brief Reads the value of a GPIO pin. Should return either 1 (HIGH) or 0 (LOW).
 */
int32_t HAL_GPIO_Read(uint16_t pin)
{
    if(PIN_MAP[pin].pin_mode == AN_INPUT)
    {
        PinMode pm = HAL_GPIO_Recall_Pin_Mode();
        if(pm == PIN_MODE_NONE)
        {
            return 0;
        }
        else
        {
            // Restore the PinMode after calling analogRead() on same pin earlier
            HAL_Pin_Mode(pin, pm);
        }
    }
    
    return nrf_gpio_pin_read(PIN_MAP[pin].gpio_pin);
}
