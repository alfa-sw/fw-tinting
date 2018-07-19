/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Humidifier Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "typedef.h"
#include "p24FJ256GB106.h"
#include "humidifierManager.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include <xc.h>

/*
*//*=====================================================================*//**
**      @brief Initialization status
**
**      @param void
**
                                                                              * 
**      @retval void
**
*//*=====================================================================*//**
*/
void initHumidifierStatusManager(void)
{
	Status.level = TINTING_INIT_ST;
}

/*
*//*=====================================================================*//**
**      @brief Humidifier Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initHumidifierParam(void)
{
    // Humidifier process Enable / Disable
	TintingAct.Humidifier_Enable = HUMIDIFIER_DISABLE;
    // Humidifier Type
	TintingAct.Humdifier_Type = HUMIDIFIER_TYPE_0;
	// Starting Humidifier Period
    TintingAct.Humidifier_Period = HUMIDIFIER_PERIOD;
	// Humidifier Nebulizer and Pump Duration with AUTOCAP OPEN
    TintingAct.AutocapOpen_Duration = AUTOCAP_OPEN_DURATION;
	// Humidifier Nebulizer and Pump Period with AUTOCAP OPEN
    TintingAct.AutocapOpen_Period = AUTOCAP_OPEN_PERIOD;
    // Temperature controlled Dosing process Enable / Disable
	TintingAct.Temp_Enable = TEMP_DISABLE;
    // Temperature Type MICROCHIP TC72
	TintingAct.Temp_Type = TEMPERATURE_TYPE_1;
	// Temperature controlled Dosing process Period 
    TintingAct.Temp_Period = TEMP_PERIOD;
	// LOW Temperature threshold value 
    TintingAct.Temp_T_LOW = TEMP_T_LOW;
	// HIGH Temperature threshold value 
    TintingAct.Temp_T_HIGH = TEMP_T_HIGH;
	// Heater Activation 
    TintingAct.Heater_Temp = HEATER_TEMP;
	// Heater Hysteresis 
    TintingAct.Heater_Hysteresis = HEATER_HYSTERESIS;

	TintingAct.Nebulizer_state = OFF;
	TintingAct.Resistance_state = OFF;
	TintingAct.Riscaldatore_state = OFF;
    TintingAct.WaterLevel_state = OFF;
    TintingAct.CriticalTemperature_state = OFF;

    Start_New_Measurement = 0;
    Sensor_Measurement_Error = FALSE;
    
    // First Temperature Value at the beginning: 25.0°C
    SHT31_Temperature = 250;
    // First Humidity Value at the beginning: 90.0%
    SHT31_Humidity = 900;
    // First Dosing Temperature Value at the beginning: 25.0°
    TC72_Temperature = 250;
    // At program start up Dosing Temperature process disabled
    TintingAct.Dosing_Temperature = 32768;
}

/*
*//*=====================================================================*//**
**      @brief Updates Humidifier status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void HumidifierManager(void)
{

}

