/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Humidifier Processes management
 * Created on 16 luglio 2018, 14.16
 */


#include "typedef.h"
#include "p24FJ256GB110.h"
#include "humidifierManager.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "stepper.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "stepperParameters.h"
#include "stepper.h"

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
	Dos_Temperature_Count_Disable_Err = 0;
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
	TintingAct.Humdifier_Type = HUMIDIFIER_TYPE_1;
    // PWM for THOR process
    TintingAct.Humidifier_PWM = HUMIDIFIER_PWM;
	// TintingAct Humidifier Period
    TintingAct.Humidifier_Period = HUMIDIFIER_PERIOD;
	// Humidifier Multiplier
    TintingAct.Humidifier_Multiplier = HUMIDIFIER_MULTIPLIER;
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

    TintingAct.WaterLevel_state = OFF;
    TintingAct.WaterPump_state = OFF;
    TintingAct.Nebulizer_Heater_state = OFF;
    TintingAct.HeaterResistance_state = OFF;  
        
    Start_New_Measurement = 0;
    Sensor_Measurement_Error = FALSE;
    
    // First Temperature Value at the beginning: 25.0�C
    SHT31_Temperature = 250;
    // First Humidity Value at the beginning: 90.0%
    SHT31_Humidity = 900;
    // First Dosing Temperature Value at the beginning: 25.0�
    TC72_Temperature = 250;
    // At program start up Dosing Temperature process disabled
    TintingAct.Dosing_Temperature = 32768;    
    impostaDuty(0);
}

/*
*//*=====================================================================*//**
**      @brief Stop Humidifier Process: All Peripherals and Sensors Acquisition
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void StopHumidifier(void)
{
	impostaDuty(0);    
	NEBULIZER_OFF();
    RISCALDATORE_OFF();
    WATER_PUMP_OFF();
    TintingAct.WaterPump_state = OFF;
    TintingAct.Nebulizer_Heater_state = OFF;
    TintingAct.HeaterResistance_state = OFF;  
/*    
    if (TintingAct.MixerMotor_state == ON) {
        TintingAct.MixerMotor_state = OFF;
        STEPPER_MIXER_OFF();        
    }        
    if (TintingAct.DoorMotor_state == ON) {
        TintingAct.DoorMotor_state = OFF;
        STEPPER_DOOR_OFF();        
    }
*/
}

/*
*//*=====================================================================*//**
**      @brief Analyze Humidifier parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
int AnalyzeHumidifierParam(void)
{
    // Humidifier process Enable / Disable
	if ( (HumidifierAct.Humidifier_Enable != HUMIDIFIER_DISABLE) && (HumidifierAct.Humidifier_Enable != HUMIDIFIER_ENABLE) )
		return FALSE;
    // Humidifier Type
	else if ( (HumidifierAct.Humdifier_Type != HUMIDIFIER_TYPE_0) && (HumidifierAct.Humdifier_Type != HUMIDIFIER_TYPE_1) && 
              (HumidifierAct.Humdifier_Type != HUMIDIFIER_TYPE_2) && (HumidifierAct.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Humidifier Multiplier
    else if ( (HumidifierAct.Humidifier_Multiplier > MAX_HUMIDIFIER_MULTIPLIER) && (HumidifierAct.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
    else if ( (HumidifierAct.AutocapOpen_Duration > MAX_HUMIDIFIER_DURATION_AUTOCAP_OPEN) && (HumidifierAct.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Period has to be >= Duration
	else if ( (HumidifierAct.AutocapOpen_Duration > HumidifierAct.AutocapOpen_Period) && (HumidifierAct.AutocapOpen_Period != 0) && 
              (HumidifierAct.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Temperature controlled Dosing process Enable / Disable
	else if ( (HumidifierAct.Temp_Enable != TEMP_DISABLE) && (HumidifierAct.Temp_Enable != TEMP_ENABLE) )
		return FALSE;
    // Temperature Type
	else if ( (HumidifierAct.Temp_Type != TEMPERATURE_TYPE_0) && (HumidifierAct.Temp_Type != TEMPERATURE_TYPE_1) && (HumidifierAct.Temp_Enable == TEMP_ENABLE) )
		return FALSE;
	// Temperature controlled Dosing process Period 
    else if ( (HumidifierAct.Temp_Period < MIN_TEMP_PERIOD) && (HumidifierAct.Temp_Enable == TEMP_ENABLE) )
		return FALSE;
	// Temperature Low threshold value has to be <= High threshold value 
    else if (HumidifierAct.Temp_T_LOW > HumidifierAct.Temp_T_HIGH)
		return FALSE;
    else
	{
		// 1sec = 500
		Durata[T_HUM_CAP_OPEN_ON] = HumidifierAct.AutocapOpen_Duration * 500;	
        Process_Period = (HumidifierAct.Humidifier_Period);
        // NO SENSOR - Process Humidifier 1.0
        if (HumidifierAct.Humdifier_Type == HUMIDIFIER_TYPE_1)
            // Initial Duration
            Durata[T_HUM_CAP_CLOSED_ON] = HumidifierAct.Humidifier_Multiplier * 500;
        // NO SENSOR - THOR process
        else if (HumidifierAct.Humdifier_Type == HUMIDIFIER_TYPE_2)
            // Initial Duration
            Durata[T_HUM_CAP_CLOSED_ON] = HumidifierAct.Humidifier_Multiplier * 500;
        else
            // Initial Duration = 2 sec
            Durata[T_HUM_CAP_CLOSED_ON] = HUMIDIFIER_DURATION * 500;	
        
        // Humidifier process Enable / Disable
        TintingAct.Humidifier_Enable = HumidifierAct.Humidifier_Enable;
        // Humidifier Type
        TintingAct.Humdifier_Type = HumidifierAct.Humdifier_Type;        
        TintingAct.Humidifier_PWM = HumidifierAct.Humidifier_PWM;
        // Humidifier Multiplier
        TintingAct.Humidifier_Multiplier = HumidifierAct.Humidifier_Multiplier;
        // Starting Humidifier Period
        TintingAct.Humidifier_Period = HumidifierAct.Humidifier_Period;
        // Humidifier Nebulizer Duration with AUTOCAP OPEN
        TintingAct.AutocapOpen_Duration = HumidifierAct.AutocapOpen_Duration;
        // Humidifier Nebulizer Period with AUTOCAP OPEN
        TintingAct.AutocapOpen_Period = HumidifierAct.AutocapOpen_Period;
        // Temperature controlled Dosing process Enable / Disable
        TintingAct.Temp_Enable = HumidifierAct.Temp_Enable;
        // Temperature Type
        TintingAct.Temp_Type = HumidifierAct.Temp_Type;
        // Temperature controlled Dosing process Period 
        TintingAct.Temp_Period = HumidifierAct.Temp_Period;
        // LOW Temperature threshold value 
        TintingAct.Temp_T_LOW = HumidifierAct.Temp_T_LOW;
        // HIGH Temperature threshold value 
        TintingAct.Temp_T_HIGH = HumidifierAct.Temp_T_HIGH;
        // Heater Activation 
        TintingAct.Heater_Temp = HumidifierAct.Heater_Temp;
        // Heater Hysteresis 
        TintingAct.Heater_Hysteresis = HumidifierAct.Heater_Hysteresis;
        
		return TRUE;
	}	
	
}

/*
*//*=====================================================================*//**
**      @brief Analyze Setup Peripheral Outputs
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
int AnalyzeSetupOutputs(void)
{    
    unsigned char count_peripheral_on;
    // Type of Peripheral
    if ( //(PeripheralAct.Peripheral_Types.MixerMotor != ON)       &&
         //(PeripheralAct.Peripheral_Types.WaterPump != ON)        &&
         (PeripheralAct.Peripheral_Types.DoorMotor != ON)        &&
         (PeripheralAct.Peripheral_Types.Nebulizer_Heater != ON) &&
         (PeripheralAct.Peripheral_Types.HeaterResistance != ON) )
		return FALSE;
    // Count Peripherals ON
    count_peripheral_on = 0;  
//    if (PeripheralAct.Peripheral_Types.MixerMotor == ON)
//        count_peripheral_on++;
//    if (PeripheralAct.Peripheral_Types.DoorMotor == ON)
//        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.WaterPump == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.Nebulizer_Heater == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.HeaterResistance == ON)
        count_peripheral_on++;
	// Only 1 Peripheral can be ON at the same time
    if (count_peripheral_on > 1)    
		return FALSE;
    
    // Peripheral Action (ON / OFF)
	if ( (TintingAct.Output_Act != OUTPUT_OFF) && (TintingAct.Output_Act != OUTPUT_ON) )
		return FALSE;
	else
		return TRUE;	
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
	static long count_dosing_period = 0;
	static long count_humidifier_period = 0;
	static long count_humidifier_period_closed = 0;
    static short int Humidifier_Count_Err, Dos_Temperature_Count_Err;
    static short int Humidifier_Count_Disable_Err;
    static short int start_timer;
	unsigned long Dos_Temperature;
	unsigned long Temperature, RH;
	static unsigned long Process_Neb_Duration;
    unsigned char count_periph_on;
    
    // Check for NEBULIZER ERRORS
#ifndef SKIP_FAULT_NEB
    if (TintingAct.Humidifier_Enable == TRUE) {
        if (isFault_Neb_Detection() && (TintingAct.Nebulizer_Heater_state == ON) ) {
            NEBULIZER_OFF();
            NextHumidifier.level = HUMIDIFIER_START;
            Humidifier.level = HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR;
        }
        else if (isFault_Neb_Detection() && (TintingAct.Nebulizer_Heater_state == OFF) ) {
            NEBULIZER_OFF();
            NextHumidifier.level = HUMIDIFIER_START;
            Humidifier.level = HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR;
        }
    }    
#endif
    // Check for WATER PUMP 
#ifndef SKIP_FAULT_PUMP
    if (TintingAct.Humidifier_Enable == TRUE) {
        if (isFault_Pump_Detection() && (TintingAct.WaterPump_state == ON) ) {
            WATER_PUMP_OFF();
            NextHumidifier.level = HUMIDIFIER_START;
            Humidifier.level = HUMIDIFIER_AIR_PUMP_OVERCURRENT_THERMAL_ERROR;
        }
        else if (isFault_Pump_Detection() && (TintingAct.WaterPump_state == OFF) ) {
            WATER_PUMP_OFF();
            NextHumidifier.level = HUMIDIFIER_START;
            Humidifier.level = HUMIDIFIER_AIR_PUMP_OPEN_LOAD_ERROR;
        }
    }    
#endif
    
  // Check for RELE
#ifndef SKIP_FAULT_RELE
    if (isFault_Rele_Detection() && (TintingAct.HeaterResistance_state == ON) && (StatusTimer(T_WAIT_RELE_TIME) == T_ELAPSED) ) {
        StopTimer(T_WAIT_RELE_TIME);                                
        StopHumidifier();
        NextHumidifier.level = HUMIDIFIER_START;        
        Humidifier.level = HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR;
    }
    else if (isFault_Rele_Detection() && (TintingAct.HeaterResistance_state == OFF) && (StatusTimer(T_WAIT_RELE_TIME) == T_ELAPSED) ) {
        StopTimer(T_WAIT_RELE_TIME);                                
        StopHumidifier();
        NextHumidifier.level = HUMIDIFIER_START;        
        Humidifier.level = HUMIDIFIER_RELE_OPEN_LOAD_ERROR;
    }  
#endif 
    
    switch(Humidifier.level)
    {
        // HUMIDIFIER IDLE
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_IDLE:
			Humidifier.level = HUMIDIFIER_START;
            StopTimer(T_HUM_CAP_OPEN_ON);
			StopTimer(T_HUM_CAP_OPEN_PERIOD);
			StopTimer(T_HUM_CAP_CLOSED_ON);
			StopTimer(T_HUM_CAP_CLOSED_PERIOD);
			StopTimer(T_DOS_PERIOD);
			Humidifier_Enable = FALSE;
			Dos_Temperature_Enable = FALSE;
            Humidifier_Count_Disable_Err = 0; 
            Dos_Temperature_Count_Disable_Err = 0;
//------------------------------------------------------------------------------            
#ifdef DEBUG_MMT
    initHumidifierParam();  
    TintingAct.Humidifier_Multiplier = HUMIDIFIER_MULTIPLIER;
    Durata[T_HUM_CAP_CLOSED_ON] = TintingAct.Humidifier_Multiplier * 500;
    Process_Period = TintingAct.Humidifier_Period;    
    // Humidifier process Enable / Disable
	TintingAct.Humidifier_Enable = HUMIDIFIER_ENABLE;
//    TintingAct.Autocap_Status = AUTOCAP_OPEN;     
	// 1sec = 500
	Durata[T_HUM_CAP_OPEN_ON] = TintingAct.AutocapOpen_Duration * 500;
	TintingAct.Temp_Enable = TEMP_ENABLE;
    // Temperature Type MICROCHIP TC72
	TintingAct.Temp_Type = TEMPERATURE_TYPE_1;
	// Temperature controlled Dosing process Period 
    TintingAct.Temp_Period = 5;	
    TintingAct.Machine_Motors = OFF; 
    TintingAct.Heater_Temp = 6;
#endif    
//------------------------------------------------------------------------------                
        break;
		// HUMIDIFIER START
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_START:
			StopHumidifier();
            Humidifier_Count_Err = 0;
            Dos_Temperature_Count_Err = 0;
            start_timer = OFF;
			if ( ((TintingAct.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingAct.Humdifier_Type == HUMIDIFIER_TYPE_0) && (Humidifier_Count_Disable_Err < HUMIDIFIER_MAX_ERROR_DISABLE))
                                                            ||
                 ((TintingAct.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingAct.Humdifier_Type == HUMIDIFIER_TYPE_1)) 
															||
                 ((TintingAct.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingAct.Humdifier_Type == HUMIDIFIER_TYPE_2)) )
            {
				Humidifier_Enable = TRUE;
				Humidifier.step = STEP_0;
				count_humidifier_period = 0;
				count_humidifier_period_closed = 0;
				Humidifier.level = HUMIDIFIER_RUNNING;
			}
            else
            {    
				Humidifier_Enable = FALSE;
                // Symbolic value that means DISABLED
                TintingAct.Temperature = 32768;
                TintingAct.RH = 32768;
            }            
            if ( (TintingAct.Temp_Enable == TEMP_ENABLE) && (Dos_Temperature_Count_Disable_Err  < DOSING_TEMPERATURE_MAX_ERROR_DISABLE) )
			{
				Dos_Temperature_Enable = TRUE;
				count_dosing_period = 0;
				StartTimer(T_DOS_PERIOD);
				Humidifier.level = HUMIDIFIER_RUNNING;			
			}
            else
			{                
				Dos_Temperature_Enable = FALSE;
                // Symbolic value that means DISABLED
                TintingAct.Dosing_Temperature = 32768;
			}                
			// Check for NEW ommmands receivd
			// ------------------------------------------------------
			// STOP command received
            if (isColorCmdStop() )
				StopHumidifier();
                
			else if (Status.level == TINTING_WAIT_PARAMETERS_ST) 
			{
				if (AnalyzeHumidifierParam() == TRUE)
				{
                    NextHumidifier.level = HUMIDIFIER_START;
                    Humidifier.level = HUMIDIFIER_PAR_RX;
				}
                else
					Humidifier.level = HUMIDIFIER_PAR_ERROR;					
			}
            
			else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_ST)
			{                
				if (AnalyzeSetupOutputs() == FALSE)
					Humidifier.level = HUMIDIFIER_PAR_ERROR;
				else 
                {
                    Humidifier.level = HUMIDIFIER_PAR_RX;
                    StopHumidifier();
                    NextHumidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}
			// ------------------------------------------------------
        break;
        // HUMIDIFIER RUNNING
		// ------------------------------------------------------------------------------------------------------------                            
        case HUMIDIFIER_RUNNING:
			// Humidifier process
			if (Humidifier_Enable == TRUE)
			{	
                // Check Water Level
				if (getWaterLevel() == OFF)
				{
					StopHumidifier();
					StopTimer(T_HUM_CAP_OPEN_ON);
					StopTimer(T_HUM_CAP_OPEN_PERIOD);
					StopTimer(T_HUM_CAP_CLOSED_ON);
					StopTimer(T_HUM_CAP_CLOSED_PERIOD);
					StopTimer(T_DOS_PERIOD);
					if (TintingAct.Temp_Enable == TEMP_ENABLE)
						StartTimer(T_DOS_PERIOD);
                    
					count_dosing_period = 0;
					count_humidifier_period = 0;
					count_humidifier_period_closed = 0;
					Humidifier.level = HUMIDIFIER_TOO_LOW_WATER_LEVEL;
				}
				else
				{	
					// Multiplier = 1000 -> Punp and Nebulizer always ON
					if (TintingAct.Humidifier_Multiplier == 1000)
					{
						TintingAct.Nebulizer_Heater_state = ON;
						if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                        {
                            NEBULIZER_ON();
                            WATER_PUMP_ON();
                            TintingAct.WaterPump_state = ON;
                        }
                        // THOR Process
                        else
                        // Nebulizer ON with PWM
                            impostaDuty(TintingAct.Humidifier_PWM);

					}
					// Multiplier = 0 --> Punp and Nebulizer always OFF
					else if (TintingAct.Humidifier_Multiplier == 0)
					{
						TintingAct.Nebulizer_Heater_state = OFF;
                        if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                        {    
                            NEBULIZER_OFF();
                            WATER_PUMP_OFF();
                            TintingAct.WaterPump_state = OFF;                            
                        }    
                        // THOR Process
                        else
                            // Nebulizer OFF
                            impostaDuty(0);                        
					}
					else
					{	                        
                        // Autocap managed by MMT --> Autocap Status is internal
                        if (TintingAct.Autocap_Enable == 1) {
                            if ( (Autocap.step == AUTOCAP_OPEN_ST) || (Autocap.step == AUTOCAP_EXTEND_RUN_ST) || (Autocap.step == AUTOCAP_EXTEND_ST) ||
                                 (Autocap.step == AUTOCAP_RETRACT_RUN_ST) || (Autocap.step == AUTOCAP_CLOSE_RUN_ST) )
                                TintingAct.Autocap_Status = AUTOCAP_OPEN;
                            else if (Autocap.step == AUTOCAP_ERROR_ST)        
                                TintingAct.Autocap_Status = AUTOCAP_ERROR;
                            else
                                TintingAct.Autocap_Status = AUTOCAP_CLOSED;                
                        }                                      
                        //  Manage Humidity Process
                        switch (TintingAct.Autocap_Status)
                        {
                            // Autocap Closed
                            case AUTOCAP_CLOSED:
                                if ( (Humidifier.step == STEP_0) || (Humidifier.step == STEP_1))
                                {	
                                    StopTimer(T_HUM_CAP_CLOSED_ON);
                                    StopTimer(T_HUM_CAP_CLOSED_PERIOD);
                                    StartTimer(T_HUM_CAP_CLOSED_ON);
                                    StartTimer(T_HUM_CAP_CLOSED_PERIOD);
                                    count_humidifier_period_closed = 0;
                                    // NEBULIZER (or HEATER Resistance) is ON at the beginning
                                    TintingAct.Nebulizer_Heater_state = ON;
									if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                        NEBULIZER_ON();
                                    // THOR Process
                                    else
                                        // Nebulizer ON with PWM
                                        impostaDuty(TintingAct.Humidifier_PWM);
                                    TintingAct.WaterPump_state = OFF;                                    
                                    Humidifier.step = STEP_2;                                    
                                }
                                else if (Humidifier.step == STEP_2)
                                {
                                    // Check Duration only NEBULIZER is Active
                                    if ( (StatusTimer(T_HUM_CAP_CLOSED_ON) == T_ELAPSED) && (TintingAct.WaterPump_state == OFF) )
                                    {
                                        StopTimer(T_HUM_CAP_CLOSED_ON);
										if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) {
                                            StartTimer(T_HUM_CAP_CLOSED_ON);
                                            WATER_PUMP_ON();
                                            TintingAct.WaterPump_state = ON;
                                        }                                            
                                        // THOR Process
                                        else 
                                            // Nebulzer OFF
                                            impostaDuty(0);
                                    } 
									// Check Duration: both NEBULIZER and PUMP are ON
                                    else if ( (StatusTimer(T_HUM_CAP_CLOSED_ON) == T_ELAPSED) && (TintingAct.WaterPump_state == ON) ) 
									{
										StopTimer(T_HUM_CAP_CLOSED_ON);
										TintingAct.Nebulizer_Heater_state = OFF;
                                        TintingAct.WaterPump_state = OFF;
                                        NEBULIZER_OFF();
                                        WATER_PUMP_OFF();
									}
                                    
                                    // Check Period
                                    if (StatusTimer(T_HUM_CAP_CLOSED_PERIOD) == T_ELAPSED) 
                                    {
                                        count_humidifier_period_closed++;
                                        StopTimer(T_HUM_CAP_CLOSED_PERIOD);				
                                        StartTimer(T_HUM_CAP_CLOSED_PERIOD);	
                                        if (count_humidifier_period_closed >= Process_Period) 
                                        {
                                            count_humidifier_period_closed = 0;    
                                            if (AcquireHumidityTemperature(TintingAct.Humdifier_Type, &Temperature, &RH) == TRUE)
                                            {
                                                TintingAct.Temperature = Temperature;
                                                TintingAct.RH = RH;
                                                HumidifierProcessCalculation(TintingAct.RH, TintingAct.Temperature, 
                                                        &Process_Period, &Process_Neb_Duration);
//    Process_Period = 30;
                                                // 1sec = 500
                                                Durata[T_HUM_CAP_CLOSED_ON] = Process_Neb_Duration;	

                                                if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                                                    NEBULIZER_ON();
                                                else 
                                                    // Nebulizer ON with PWM
                                                    impostaDuty(TintingAct.Humidifier_PWM);

                                                StopTimer(T_HUM_CAP_CLOSED_ON);
                                                StartTimer(T_HUM_CAP_CLOSED_ON);
                                                // Only NEBULIZER is ON at the beginning
                                                TintingAct.Nebulizer_Heater_state = ON;
                                                TintingAct.WaterPump_state = OFF;
                                            }                                         
                                            else
                                            {	
    //                                          StopTimer(T_HUM_CAP_CLOSED_PERIOD);
                                                Humidifier_Count_Err++;
                                                Humidifier_Count_Disable_Err++;
                                                TintingAct.Nebulizer_Heater_state = OFF;
                                                if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) {
                                                    TintingAct.WaterPump_state = OFF;                                                    
                                                    WATER_PUMP_OFF();
                                                    NEBULIZER_OFF();                                            
                                                }  
                                                // THOR Process
                                                else 
                                                    // Nebulzer OFF
                                                    impostaDuty(0);
                                                
                                                if (Humidifier_Count_Err >= HUMIDIFIER_MAX_ERROR)
                                                {
                                                    // Symbolic value that means DISABLED
                                                    TintingAct.Temperature = 32768;
                                                    TintingAct.RH = 32768;                
                                                    Humidifier_Enable = FALSE;
                                                }
                                                NextHumidifier.level = HUMIDIFIER_RUNNING;
                                                Humidifier.level = HUMIDIFIER_RH_ERROR;
                                            }
                                        }										
                                    }
                                }    
                            break;

                            // Autocap Open
                            case AUTOCAP_OPEN:
                                // Period = 0 -> Nebulizer (or Heater) always ON
                                if (TintingAct.AutocapOpen_Period == 0)
                                {
                                    TintingAct.Nebulizer_Heater_state = ON;
                                    if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)  {
                                        TintingAct.WaterPump_state = ON;
                                        WATER_PUMP_ON();
                                        NEBULIZER_ON();
                                    } 
                                    // THOR Process
                                    else
                                        // Nebulizer ON with PWM
                                        impostaDuty(TintingAct.Humidifier_PWM);
                                }
                                // Duration = 0 AND Period != 0 -> Nebulizer (or Heater) always OFF
                                else if (TintingAct.AutocapOpen_Duration == 0)
                                {
                                    TintingAct.Nebulizer_Heater_state = OFF;
                                    if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                        NEBULIZER_OFF();
                                    // THOR Process
                                    else
                                        // Nebulizer OFF
                                        impostaDuty(0); 
                                    TintingAct.WaterPump_state = OFF;
                                    WATER_PUMP_OFF();                                    
                                }
                                // Duration > 0 AND Period > 0
                                else
                                {
                                    // Initialization
                                    if ( (Humidifier.step == STEP_0) || (Humidifier.step == STEP_2) )
                                    {	
                                        StopTimer(T_HUM_CAP_OPEN_ON);
                                        StopTimer(T_HUM_CAP_OPEN_PERIOD);
                                        StartTimer(T_HUM_CAP_OPEN_ON);
                                        StartTimer(T_HUM_CAP_OPEN_PERIOD);
                                        count_humidifier_period = 0;
                                        TintingAct.Nebulizer_Heater_state = ON;
                                        if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) {
                                            TintingAct.WaterPump_state = ON;
                                            WATER_PUMP_ON();                                                                                
                                            NEBULIZER_ON();                                        
                                        }
                                            // THOR Process
                                        else
                                            // Nebulizer ON with PWM
                                            impostaDuty(TintingAct.Humidifier_PWM);
                                        
                                        Humidifier.step = STEP_1;
                                    }
                                    else if (Humidifier.step == STEP_1)
                                    {
                                        // Check Duration
                                        if (StatusTimer(T_HUM_CAP_OPEN_ON) == T_ELAPSED)
                                        {
                                            StopTimer(T_HUM_CAP_OPEN_ON);
                                            TintingAct.Nebulizer_Heater_state = OFF;
                                            if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                                NEBULIZER_OFF();
                                            // THOR Process
                                            else
                                                // Nebulizer OFF
                                                impostaDuty(0);   
                                            TintingAct.WaterPump_state = OFF;
                                            WATER_PUMP_OFF();                                                                                
                                        }
                                        // Check Period
                                        if (StatusTimer(T_HUM_CAP_OPEN_PERIOD) == T_ELAPSED) 
                                        {
                                            count_humidifier_period++;
                                            StopTimer(T_HUM_CAP_OPEN_PERIOD);				
                                            StartTimer(T_HUM_CAP_OPEN_PERIOD);	
                                            if (count_humidifier_period == TintingAct.AutocapOpen_Period) 
                                            {
                                                StartTimer(T_HUM_CAP_OPEN_ON);
                                                count_humidifier_period = 0;
                                                TintingAct.Nebulizer_Heater_state = ON;
                                                if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) {
                                                    NEBULIZER_ON();
                                                    TintingAct.WaterPump_state = ON;   
                                                    WATER_PUMP_ON();                                                                                                                                    
                                                }    
                                                // THOR Process
                                                else
                                                    // Nebulizer ON with PWM
                                                    impostaDuty(TintingAct.Humidifier_PWM);                                                                                                
                                            }
                                        }										
                                    }							
                                }
                            break;

                            // Autocap Error				
                            case AUTOCAP_ERROR:
                                StopHumidifier();
                                Humidifier_Enable = FALSE;
                                if (Dos_Temperature_Enable == TRUE)
                                {	
                                    count_dosing_period = 0;
                                    StopTimer(T_DOS_PERIOD);
                                    StartTimer(T_DOS_PERIOD);
                                }								
                            break;

                            default:
                               break;
                        }
                    }
				}	
			}
			// Dosing Temperature process
			if (Dos_Temperature_Enable == TRUE)
			{
				if (StatusTimer(T_DOS_PERIOD) == T_ELAPSED) 
				{
					count_dosing_period++;
					StopTimer(T_DOS_PERIOD);			
					StartTimer(T_DOS_PERIOD);							
					if (count_dosing_period >= TintingAct.Temp_Period) 
					{
						count_dosing_period = 0;
                        
						if (AcquireTemperature(TintingAct.Temp_Type, &Dos_Temperature) == TRUE)
						{
                            TintingAct.Dosing_Temperature = Dos_Temperature;
                            Dos_Temperature_Count_Err = 0;
                            TintingAct.Dosing_Temperature = Dos_Temperature;
                            if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Machine_Motors == TRUE) || (Status.level == TINTING_JAR_MOTOR_RUN_ST) || 
                                  (Status.level == TINTING_SUPPLY_RUN_ST) || (Mixer.phase == MIXER_PHASE_MIXING) || (Mixer.phase == MIXER_PHASE_DOOR_OPEN) || (Mixer.phase == MIXER_PHASE_HOMING)) ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }                                                            
                            else if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Dosing_Temperature/10) >= (unsigned long)(TintingAct.Heater_Temp + TintingAct.Heater_Hysteresis) ) )
                            {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }
                            else if  ( (TintingAct.HeaterResistance_state == OFF)  && (TintingAct.Machine_Motors == FALSE) && (Status.level != TINTING_JAR_MOTOR_RUN_ST) && (Status.level != TINTING_SUPPLY_RUN_ST) &&
                                       (Mixer.phase != MIXER_PHASE_MIXING) && (Mixer.phase != MIXER_PHASE_DOOR_OPEN) && (Mixer.phase != MIXER_PHASE_HOMING) && ((TintingAct.Dosing_Temperature/10) <= (unsigned long)(TintingAct.Heater_Temp - TintingAct.Heater_Hysteresis) ) )
                            {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = ON;          
                                RISCALDATORE_ON();
                            }
                        }    
                        else
						{
//							StopTimer(T_DOS_PERIOD);
                            Dos_Temperature_Count_Err++;
                            if (Dos_Temperature_Count_Err >= DOSING_TEMPERATURE_MAX_ERROR)
                            {    
                                Dos_Temperature_Count_Err = 0;
                                Dos_Temperature_Count_Disable_Err++;
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();                                                        
                                // Symbolic value that means DISABLED
                                TintingAct.Dosing_Temperature = 32768;                                
                                Dos_Temperature_Enable = FALSE;
                                NextHumidifier.level = HUMIDIFIER_RUNNING;
                                Humidifier.level     = HUMIDIFIER_TEMPERATURE_ERROR;                                
                            }                
                        }
					}
                    else {
                        if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Machine_Motors == TRUE) || (Status.level == TINTING_JAR_MOTOR_RUN_ST) || 
                              (Status.level == TINTING_SUPPLY_RUN_ST) || (Mixer.phase == MIXER_PHASE_MIXING) || (Mixer.phase == MIXER_PHASE_DOOR_OPEN) || (Mixer.phase == MIXER_PHASE_HOMING)) ) {                      
                            StopTimer(T_WAIT_RELE_TIME);                                
                            TintingAct.HeaterResistance_state = OFF;
                            RISCALDATORE_OFF();
                        }                                
                    }                                         
				}					
			}	
			// Check for NEW ommmands receivd
			// ------------------------------------------------------
			// STOP command received
            if (isColorCmdStop() )
                StopHumidifier();
                
			else if (Status.level == TINTING_WAIT_PARAMETERS_ST) 
			{
				if (AnalyzeHumidifierParam() == TRUE)
				{
                    NextHumidifier.level = HUMIDIFIER_START;
                    Humidifier.level = HUMIDIFIER_PAR_RX;
				}
                else
					Humidifier.level = HUMIDIFIER_PAR_ERROR;					
			}
            
			else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_ST)
			{                
				if (AnalyzeSetupOutputs() == FALSE)
					Humidifier.level = HUMIDIFIER_PAR_ERROR;
				else 
                {
                    Humidifier.level = HUMIDIFIER_PAR_RX;
                    StopHumidifier();
                    NextHumidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}
			// ------------------------------------------------------
				
		break;		
		// PERIPHERAL OUTPUT MANAGER
		// ------------------------------------------------------------------------------------------------------------
        case HUMIDIFIER_SETUP_OUTPUT:
			// A 'TintingAct.Output_Act' == OUTPUT_OFF' is arrived
            if (Status.level == TINTING_WAIT_SETUP_OUTPUT_ST)
			{
				if (AnalyzeSetupOutputs() == FALSE)
					Humidifier.level = HUMIDIFIER_PAR_ERROR;
				else 
                {
                    Humidifier.level = HUMIDIFIER_PAR_RX;
                    NextHumidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}
/*            
			// STOP PROCESS command received
            if (isColorCmdStop() ) {
				StopHumidifier();
                Humidifier.level = HUMIDIFIER_START;
            }
*/ 
            // RESET command received
            else if (isColorCmdIntr() ) {
				StopHumidifier();
                Humidifier.level = HUMIDIFIER_START;
            }
            // Water Pump    
            else if (PeripheralAct.Peripheral_Types.WaterPump == ON) {        
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    TintingAct.WaterPump_state = ON;
                    WATER_PUMP_ON();
                }    
                else {
                    TintingAct.WaterPump_state = OFF;
                    WATER_PUMP_OFF();
                }    
            }
            // Nebulizer or Heater
            else if (PeripheralAct.Peripheral_Types.Nebulizer_Heater == ON) { 
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    TintingAct.Nebulizer_Heater_state = ON;
					if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2)
                        NEBULIZER_ON();    
                    // THOR Process
                    else
                    // Nebulizer ON with PWM
                        impostaDuty(TintingAct.Humidifier_PWM);                    
                }    
                else {
                    TintingAct.Nebulizer_Heater_state = OFF;
                    if (TintingAct.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                        NEBULIZER_OFF();                        
                    // THOR Process
                    else
                    // Nebulizer OFF
                        impostaDuty(0);                                           
                }    
            }
            // Heater Resistance    
            else if (PeripheralAct.Peripheral_Types.HeaterResistance == ON) {
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    TintingAct.HeaterResistance_state = ON;
                    RISCALDATORE_ON();
                }    
                else {
                    TintingAct.HeaterResistance_state = OFF;
                    RISCALDATORE_OFF();
                }    
            }
/*            
            // Mixer Motor    
            else if (PeripheralAct.Peripheral_Types.MixerMotor == ON) {
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {  
                        TintingAct.MixerMotor_state = ON;
                        STEPPER_MIXER_ON();                    
                    }
                    else {
                        TintingAct.MixerMotor_state = OFF;
                        STEPPER_MIXER_OFF();
                    }    
                }    
                else {
                    TintingAct.MixerMotor_state = OFF;
                    STEPPER_MIXER_OFF();
                }    
            }
*/
/*
            // Door Motor    
            else if (PeripheralAct.Peripheral_Types.DoorMotor == ON) {
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    TintingAct.DoorMotor_state = ON;
                    STEPPER_DOOR_ON();
                }    
                else {
                    TintingAct.DoorMotor_state = OFF;
                    STEPPER_DOOR_OFF();
                }    
            }
*/                        
            // Count Peripherals ON
            count_periph_on = 0;
/*
            if (TintingAct.DoorMotor_state == ON)
                count_periph_on++;            
            if (TintingAct.MixerMotor_state == ON)
                count_periph_on++;
*/
            if (TintingAct.WaterPump_state == ON)
                count_periph_on++;
            if (TintingAct.Nebulizer_Heater_state == ON)
                count_periph_on++;
            if (TintingAct.HeaterResistance_state == ON)
                count_periph_on++;
            // All the peripherals turned OFF -->  Go to initial Status            
            if (count_periph_on == 0 )
                Humidifier.level = HUMIDIFIER_START;
        break;
		// HUMIDIFIER PARAMETERS ARE CORRECT		
		// ------------------------------------------------------------------------------------------------------------
		case HUMIDIFIER_PAR_RX:
			if ( (Status.level != TINTING_WAIT_SETUP_OUTPUT_ST) &&
                 (Status.level != TINTING_WAIT_PARAMETERS_ST) )   
                Humidifier.level = NextHumidifier.level;
			// STOP command received
            else if (isColorCmdStop() ) { 
				StopHumidifier();
                Humidifier.level = HUMIDIFIER_START;
			}                        
        break;
        
        // HUMIDIFIER TOO LOW WATER LEVEL
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_TOO_LOW_WATER_LEVEL:
			if (getWaterLevel() == ON) {
				// Gestione Lampeggio LED
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				Humidifier.level = HUMIDIFIER_START;
			}
			// Manage Dosing Temperature process if activated
			if (TintingAct.Temp_Enable == TEMP_ENABLE) 
			{
				if (StatusTimer(T_DOS_PERIOD) == T_ELAPSED) 
				{
					count_dosing_period++;
					StopTimer(T_DOS_PERIOD);			
					StartTimer(T_DOS_PERIOD);							
					if (count_dosing_period == TintingAct.Temp_Period) 
					{
						count_dosing_period = 0;
						if (AcquireTemperature(TintingAct.Temp_Type, &Dos_Temperature) == TRUE)
						{
                            TintingAct.Dosing_Temperature = Dos_Temperature;
/*
if (TintingAct.Dosing_Temperature == 250)
    TintingAct.Dosing_Temperature = 40;
else if (TintingAct.Dosing_Temperature == 40)                            
    TintingAct.Dosing_Temperature = 250;
else if (TintingAct.Dosing_Temperature == 32768)
    TintingAct.Dosing_Temperature = 40;
*/
                            Dos_Temperature_Count_Err = 0;
                            TintingAct.Dosing_Temperature = Dos_Temperature;
                            if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Machine_Motors == TRUE) || (Status.level == TINTING_JAR_MOTOR_RUN_ST) || 
                                  (Status.level == TINTING_SUPPLY_RUN_ST) )   ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }                                                            
                            else if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Dosing_Temperature/10) >= (unsigned long)(TintingAct.Heater_Temp + TintingAct.Heater_Hysteresis) ) )
                            {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }
                            else if  ( (TintingAct.HeaterResistance_state == OFF)  && (TintingAct.Machine_Motors == FALSE) && (Status.level != TINTING_JAR_MOTOR_RUN_ST) && (Status.level != TINTING_SUPPLY_RUN_ST) &&
                                       (Mixer.phase != MIXER_PHASE_MIXING) && (Mixer.phase != MIXER_PHASE_DOOR_OPEN) && (Mixer.phase != MIXER_PHASE_HOMING) && ((TintingAct.Dosing_Temperature/10) <= (unsigned long)(TintingAct.Heater_Temp - TintingAct.Heater_Hysteresis) ) )
                            {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = ON;          
                                RISCALDATORE_ON();
                            }                            
                        }    
						else
						{                            
//							StopTimer(T_DOS_PERIOD);
                            Dos_Temperature_Count_Err++;
                            if (Dos_Temperature_Count_Err >= DOSING_TEMPERATURE_MAX_ERROR)
                            {    
                                Dos_Temperature_Count_Err = 0;
                                Dos_Temperature_Count_Disable_Err++;
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();                                                        
                                // Symbolic value that means DISABLED
                                TintingAct.Dosing_Temperature = 32768;                                
                                Dos_Temperature_Enable = FALSE;
                                NextHumidifier.level = HUMIDIFIER_RUNNING;
                                Humidifier.level     = HUMIDIFIER_TEMPERATURE_ERROR;                                
                            }                
						}
					}
                    else {
                        if ( (TintingAct.HeaterResistance_state == ON) && ((TintingAct.Machine_Motors == TRUE) || (Status.level == TINTING_JAR_MOTOR_RUN_ST) || 
                              (Status.level == TINTING_SUPPLY_RUN_ST) || (Mixer.phase == MIXER_PHASE_MIXING) || (Mixer.phase == MIXER_PHASE_DOOR_OPEN) || (Mixer.phase == MIXER_PHASE_HOMING)) ) {
                            StopTimer(T_WAIT_RELE_TIME);                                
                            TintingAct.HeaterResistance_state = OFF;
                            RISCALDATORE_OFF();
                        }                                
                    }                                                             
				}					
			}

			// Check for NEW ommmands received
			// ------------------------------------------------------
			// STOP PROCESS command received
			if (isColorCmdStop() )
			{
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				StopHumidifier();
				Humidifier.level = HUMIDIFIER_START;
			}
			else if (Status.level == TINTING_WAIT_PARAMETERS_ST) 
			{
				if (AnalyzeHumidifierParam() == TRUE)
				{
                    NextHumidifier.level = HUMIDIFIER_START;
                    Humidifier.level = HUMIDIFIER_PAR_RX;
				}
                else
					Humidifier.level = HUMIDIFIER_PAR_ERROR;					
			}
			else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_ST)
			{                
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				if (AnalyzeSetupOutputs() == FALSE)
					Humidifier.level = HUMIDIFIER_PAR_ERROR;
				else 
                {
                    Humidifier.level = HUMIDIFIER_PAR_RX;
                    StopHumidifier();
                    NextHumidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}            
        break;

		// HUMIDIFIER ERROR
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_RH_ERROR:
        case HUMIDIFIER_TEMPERATURE_ERROR:
        case HUMIDIFIER_PAR_ERROR: 
        case HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR:
        case HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR:
        case HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR:
        case HUMIDIFIER_RELE_OPEN_LOAD_ERROR: 
        case HUMIDIFIER_AIR_PUMP_OVERCURRENT_THERMAL_ERROR:
        case HUMIDIFIER_AIR_PUMP_OPEN_LOAD_ERROR:  
            if ( (Dos_Temperature_Enable == TRUE) || (Humidifier_Enable == TRUE) ) 
            {
                // Wait a period before to come to previous Status
                if (start_timer == OFF)
                {    
                    StopTimer(T_ERROR_STATUS);
                    StartTimer(T_ERROR_STATUS);
                    start_timer = ON;                
                }
                else if (StatusTimer(T_ERROR_STATUS) == T_ELAPSED)
                {    
                    StopTimer(T_ERROR_STATUS);
                    start_timer = OFF;
                    Humidifier.level = NextHumidifier.level;
                }
            }
            // Both processes Humidifier and Dosing temperature are Disabled
            // Error condition detected, Stop Nebulizer, Pump and Sensor Acquisition
            else    
                StopHumidifier();

			// Check for NEW ommmands receivd
			// ------------------------------------------------------
			if (Status.level == TINTING_WAIT_PARAMETERS_ST) 
            {
				if (AnalyzeHumidifierParam() == TRUE)
				{
                    NextHumidifier.level = HUMIDIFIER_START;
                    Humidifier.level = HUMIDIFIER_PAR_RX;
				}
                else
					Humidifier.level = HUMIDIFIER_PAR_ERROR;					
			}            
			else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_ST) 
            {
				if (AnalyzeSetupOutputs() == FALSE)
					Humidifier.level = HUMIDIFIER_PAR_ERROR;
				else 
                {
                    Humidifier.level = HUMIDIFIER_PAR_RX;
                    StopHumidifier();
                    NextHumidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}            
        break;
		// ------------------------------------------------------------------------------------------------------------        
        
        default:
            Humidifier.level = HUMIDIFIER_IDLE;             
        break;            
    }        
}

/*
*//*=====================================================================*//**
**      @brief Temperature Measurement
**
**      @param unsigned char Temp_Type --> Type of Temperature Sensor
**			   unsigned long *Temp	   --> Temperature Measurement
**
**      @retval bool --> TRUE  = good measurement
**					 --> FALSE = bad measurement
**
*//*=====================================================================*//**
*/
int AcquireTemperature(unsigned char Temp_Type, unsigned long *Temp)
{
	switch (Temp_Type)
	{
		// SHT31
		case TEMPERATURE_TYPE_0:
            // Humidifier process Not Active: a new measurement is performed
            if (Humidifier_Enable == FALSE)
            {    
                if (Start_New_Measurement == OFF) 
                    Start_New_Measurement = ON;
                if (Sensor_Measurement_Error == FALSE)
                {
                    *Temp = SHT31_Temperature;
                    return TRUE;
                }
                else
                    return FALSE;
            }
            // Humidifier process Active: it is used the valure obtained from that measuremente
            else
            {
                *Temp = SHT31_Temperature;
    			return TRUE;
            }
        break;
		// Sensore Temperatura Microchip TC72
		case TEMPERATURE_TYPE_1:            
			if (Start_New_Temp_Measurement == OFF) 
                Start_New_Temp_Measurement = ON;
            if (Sensor_Temp_Measurement_Error == FALSE)
            {
                *Temp = TC72_Temperature;
    			return TRUE;
            }
            else
    			return FALSE; 
//            *Temp = 80;;    
    		return TRUE;            
        break;
		
		default:
			return FALSE;
	}	
}

/*
*//*=====================================================================*//**
**      @brief Humidity Temperature Measurement
**
**      @param unsigned char Temp_Type --> Type of Sensor: 0 = Sensirion SHT31, 1 = NO sensor, 2 = No sensor
**			   unsigned long *Temp	   --> Temperature Measurement
**
**      @retval bool --> TRUE  = good measurement
**					 --> FALSE = bad measurement
**
*//*=====================================================================*//**
*/
int AcquireHumidityTemperature(unsigned char Temp_Type, unsigned long *Temp, unsigned long *Humidity)
{
	switch (Temp_Type)
	{
		// SHT31
		case 0:            
			if (Start_New_Measurement == OFF) 
                Start_New_Measurement = ON;
            if (Sensor_Measurement_Error == FALSE)
            {
                *Temp = SHT31_Temperature;
                *Humidity = SHT31_Humidity;
    			return TRUE;
            }
            else
    			return FALSE;
//    	return TRUE;
//    	return FALSE;
            break;

		// NO SENSOR
		case 1: 
        case 2:    
            *Temp = 32768;
            *Humidity = 32768;        
            return TRUE;
//    	return FALSE;
            break;
            
		default:
			return FALSE;
	}	
}

/*
*//*=====================================================================*//**
**      @brief Humidity Temperature Measurement
**
**      @param 
**			   unsigned long RH --> RH Humiity 			   
**			   unsigned long Temperature --> Temperature x 10 
**
**			   unsigned long Period --> Period of Humidifier Process (unit 1=1sec)
**			   unsigned long Pump_Duration --> Pump activation Duration of Humidifier Process (unit 1=2msec)
**			   unsigned long Neb_Duration --> Nebulizer activation Duration of Humidifier Process (unit 1=2msec)				
**
**      @retval void
**					 
**
*//*=====================================================================*//**
*/
void HumidifierProcessCalculation(unsigned long RH, unsigned long Temperature, unsigned long *Period, unsigned long *Neb_Duration)
{
	float KT1, KH1, KT2, KH2, KBoostT, KBoostH, KBoostTH, KRT, KRH, Temp, RH_Humidity, Knormalizz, Period_calc;
	
    // NO SENSOR - Process Humidifier 1.0
    if (TintingAct.Humdifier_Type == HUMIDIFIER_TYPE_1)
    {
        // 1 = 1sec
        *Period = TintingAct.Humidifier_Period;  
        // 1 = 2msec
        *Neb_Duration = TintingAct.Humidifier_Multiplier * 500;
    }
    // NO SENSOR - THOR process
    else if (TintingAct.Humdifier_Type == HUMIDIFIER_TYPE_2)
    {
        // 1 = 1sec
        *Period = TintingAct.Humidifier_Period;  
        // 1 = 2msec
        *Neb_Duration = TintingAct.Humidifier_Multiplier * 500;
    }     
    // SENSOR SHT31 - Process Humidifier 2.0
    else
    {    
        Temp = (float)Temperature/10;
        RH_Humidity = (float)RH/1000;
//	Temp = (float)20;
//	RH_Humidity = (float)0.5;

        // Normalization factor betwee old system and NEW one x conversion factor between sec to 2msec unit (= 0.007692 * 500)
        Knormalizz = 3.846;

        // KT1 = variabile di temperatura per definizione del "Period"
        // Calcolo KT1
        if (Temp < 10)
            KT1 = 0.75;
        else if (Temp > 30)
            KT1 = 0.25;
        else
            KT1 = 0.5 + (20 - Temp) * 0.025;

        // KH1 = variabile di umidita'� per definizione del "Period"
        // Calcolo KH1
        if (RH_Humidity < 0.4)
            KH1 = 0.25;
        else if (RH_Humidity > 0.8)
            KH1 = 0.75;
        else
            KH1 = 0.5 -(0.6 - RH_Humidity) * 5 / 4;

        // "Period = tempo all'interno del quale si calcola il tempo di funzionamento della Pompa e del Nebulizzatore dell'acuqa della bottiglia
        // Calcolo "Period"
        *Period = TintingAct.Humidifier_Period * (KT1 + KH1);
        Period_calc = (float)TintingAct.Humidifier_Period * (KT1 + KH1);

        // KT2 = variabile di temperatura per definizione di "Pump_Duration"
        // Calcolo KT2
        if (Temp < 10)
            KT2 = 0.5;
        else if (Temp > 30)
            KT2 = 1.5;
        else
            KT2 = 0.5 + (Temp - 10) / 20;

        // KH2 = variabile di umidità per definizione del "Pump_Duration"
        // Calcolo KH2
        if (RH_Humidity < 0.4)
            KH2 = 0.6;
        else if (RH_Humidity > 0.8)
            KH2 = 0.2;
        else
            KH2 = 1 - RH_Humidity;

        // KBoostT = variabile di temperatura per aumentare il "Period" in un determinato range di T
        // Calcolo KBoostT
        if (Temp < 30)
            KBoostT = 0.25;
        else if (Temp > 40)
            KBoostT = 0.5;
        else
            KBoostT = 0.25 + (Temp - 30) / 40;

        // KBoostH = variabile di umidità per aumentare il "Period" in un determinato range di H
        // Calcolo KBoostH 
        if (RH_Humidity < 0.2)
            KBoostH = 0.75;
        else if (RH_Humidity > 0.4)
            KBoostH = 0.25;
        else
            KBoostH = 0.25 + (0.4 - RH_Humidity) * 2.5;

        // Calcolo KBoostTH
        KBoostTH = KBoostT + KBoostH;

        // KRT = variabile di temperatura per aumentare "Neb_Duration" in un determinato range di T
        // Calcolo KRT 
        if (Temp < 30)
            KRT = 0.0;
        else if (Temp > 40)
            KRT = 0.45;
        else
            KRT = (Temp - 30) * 4.5 / 100;

        // KRH = variabile di umidità per aumentare il "Neb_Duration" in un determinato range di H
        // Calcolo KRH 
        if (RH_Humidity < 0.2)
            KRH = 0.45;
        else if (RH_Humidity > 0.5)
            KRH = 0.0;
        else
            KRH = (0.5 - RH_Humidity) * 1.5;

        *Neb_Duration = Period_calc * KT2 * KH2 * KBoostTH * Knormalizz;
        if ((*Neb_Duration / 500) > *Period)
            *Neb_Duration = *Period * 500;

        // "Neb_Duration" = tempo durante il quale il Nebulizzatore o la Resistenza dell'acqua della bottiglia rimane acceso
        // Calcolo "Neb_Duration"
        *Neb_Duration = *Neb_Duration * (0.1 + KRT + KRH);
    }         
}

/*
*//*=====================================================================*//**
**      @brief Duty Cycle
**
**      @param 
**
**      @retval void
**					 
**
*//*=====================================================================*//**
*/
void impostaDuty(char val)
{
	if (val > 50)
		val = 0;
	IEC0bits.T1IE = 0; // Disable Timer1 Interrupt
	dutyPWM = val;
	IEC0bits.T1IE = 1; // Enable Timer1 Interrupt    
}
