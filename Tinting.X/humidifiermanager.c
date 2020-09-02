/* 
 * File:   humidifiermanager.h
 * Author: michele.abelli
 * Description: Humidifier Processes management
 * Created on 16 luglio 2018, 14.16
 */


#include "typedef.h"
#include "p24FJ256GB110.h"
#include "humidifierManager.h"
#include "tintingManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "stepper.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "autocapAct.h"
#include "errorManager.h"
#include "spi.h"
#include "colorAct.h"
#include "eepromManager.h"
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
	Humidifier.level = HUMIDIFIER_IDLE;
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
	TintingHumidifier.Humidifier_Enable = HUMIDIFIER_DISABLE;
    // Humidifier Type
	TintingHumidifier.Humdifier_Type = HUMIDIFIER_TYPE_0;
    // PWM for THOR process
    TintingHumidifier.Humidifier_PWM = HUMIDIFIER_PWM;
	// TintingAct Humidifier Period
    TintingHumidifier.Humidifier_Period = HUMIDIFIER_PERIOD;
	// Humidifier Multiplier
    TintingHumidifier.Humidifier_Multiplier = HUMIDIFIER_MULTIPLIER;
	// Humidifier Nebulizer and Pump Duration with AUTOCAP OPEN
    TintingHumidifier.AutocapOpen_Duration = AUTOCAP_OPEN_DURATION;
	// Humidifier Nebulizer and Pump Period with AUTOCAP OPEN
    TintingHumidifier.AutocapOpen_Period = AUTOCAP_OPEN_PERIOD;
    // Temperature controlled Dosing process Enable / Disable
	TintingHumidifier.Temp_Enable = TEMP_DISABLE;
    // Temperature Type MICROCHIP TC72
	TintingHumidifier.Temp_Type = TEMPERATURE_TYPE_1;
	// Temperature controlled Dosing process Period 
    TintingHumidifier.Temp_Period = TEMP_PERIOD;
	// LOW Temperature threshold value 
    TintingHumidifier.Temp_T_LOW = TEMP_T_LOW;
	// HIGH Temperature threshold value 
    TintingHumidifier.Temp_T_HIGH = TEMP_T_HIGH;
	// Heater Activation 
    TintingHumidifier.Heater_Temp = HEATER_TEMP;
	// Heater Hysteresis 
    TintingHumidifier.Heater_Hysteresis = HEATER_HYSTERESIS;

    TintingAct.WaterLevel_state = OFF;
    TintingAct.CriticalTemperature_state = OFF;
    
    //TintingAct.RotatingTable_state = OFF;
    TintingAct.Brush_state = OFF;
    TintingAct.WaterPump_state = OFF;
    TintingAct.Nebulizer_Heater_state = OFF;
    TintingAct.HeaterResistance_state = OFF;  
    //TintingAct.OpenValve_BigHole_state = OFF;  
    //TintingAct.OpenValve_SmallHole_state = OFF;  
        
    Start_New_Measurement = 0;
    Sensor_Measurement_Error = FALSE;
    
    // First Temperature Value at the beginning: 25.0∞C
    SHT31_Temperature = 250;
    // First Humidity Value at the beginning: 90.0%
    SHT31_Humidity = 900;
    // First Dosing Temperature Value at the beginning: 25.0∞
    TC72_Temperature = 250;
    // At program start up Dosing Temperature process disabled
    TintingAct.Dosing_Temperature = DOSING_TEMP_PROCESS_DISABLED; 
#ifndef CAR_REFINISHING_MACHINE    
    impostaDuty(0);
#endif    
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
#ifndef CAR_REFINISHING_MACHINE       
	impostaDuty(0); 
	NEBULIZER_OFF();
#endif    
    RISCALDATORE_OFF();
//	StopSensor();
    TintingAct.Brush_state = OFF;
    TintingAct.WaterPump_state = OFF;
    TintingAct.Nebulizer_Heater_state = OFF;
    TintingAct.HeaterResistance_state = OFF;  
    //TintingAct.OpenValve_BigHole_state = OFF;  
    //TintingAct.OpenValve_SmallHole_state = OFF;
    TintingAct.Dosing_Temperature = DOSING_TEMP_PROCESS_DISABLED;
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
	if ( (TintingHumidifier.Humidifier_Enable != HUMIDIFIER_DISABLE) && (TintingHumidifier.Humidifier_Enable != HUMIDIFIER_ENABLE) )
		return FALSE;
    // Humidifier Type
	else if ( (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_0) && (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_1) && 
              (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) && (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Humidifier Multiplier
    else if ( (TintingHumidifier.Humidifier_Multiplier > MAX_HUMIDIFIER_MULTIPLIER) && (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
    else if ( (TintingHumidifier.AutocapOpen_Duration > MAX_HUMIDIFIER_DURATION_AUTOCAP_OPEN) && (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Period has to be >= Duration
	else if ( (TintingHumidifier.AutocapOpen_Duration > TintingHumidifier.AutocapOpen_Period) && (TintingHumidifier.AutocapOpen_Period != 0) && 
              (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) )
		return FALSE;
	// Temperature controlled Dosing process Enable / Disable
	else if ( (TintingHumidifier.Temp_Enable != TEMP_DISABLE) && (TintingHumidifier.Temp_Enable != TEMP_ENABLE) )
		return FALSE;
    // Temperature Type
	else if ( (TintingHumidifier.Temp_Type != TEMPERATURE_TYPE_0) && (TintingHumidifier.Temp_Type != TEMPERATURE_TYPE_1) && (TintingHumidifier.Temp_Enable == TEMP_ENABLE) )
		return FALSE;
	// Temperature controlled Dosing process Period 
    else if ( (TintingHumidifier.Temp_Period < MIN_TEMP_PERIOD) && (TintingHumidifier.Temp_Enable == TEMP_ENABLE) )
		return FALSE;
	// Temperature Low threshold value has to be <= High threshold value 
    else if (TintingHumidifier.Temp_T_LOW > TintingHumidifier.Temp_T_HIGH)
		return FALSE;
    else
	{
		// 1sec = 5000
		Durata[T_HUM_CAP_OPEN_ON] = TintingHumidifier.AutocapOpen_Duration * 5000;	
        Process_Period = (TintingHumidifier.Humidifier_Period);
        // NO SENSOR - Process Humidifier 1.0
        if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_1)
            // Initial Duration
            Durata[T_HUM_CAP_CLOSED_ON] = TintingHumidifier.Humidifier_Multiplier * 5000;
        // NO SENSOR - THOR process
        else if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2)
            // Initial Duration
            Durata[T_HUM_CAP_CLOSED_ON] = TintingHumidifier.Humidifier_Multiplier * 5000;
        else
            // Initial Duration = 2 sec
            Durata[T_HUM_CAP_CLOSED_ON] = HUMIDIFIER_DURATION * 5000;	

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
    if ( (PeripheralAct.Peripheral_Types.RotatingTable != ON)    &&
         (PeripheralAct.Peripheral_Types.Cleaner != ON)          &&
         (PeripheralAct.Peripheral_Types.WaterPump != ON)        &&
         (PeripheralAct.Peripheral_Types.Nebulizer_Heater != ON) &&
         (PeripheralAct.Peripheral_Types.HeaterResistance != ON) &&
         (PeripheralAct.Peripheral_Types.OpenValve_BigHole != ON)&&
         (PeripheralAct.Peripheral_Types.OpenValve_SmallHole != ON) &&
         (PeripheralAct.Peripheral_Types.Rotating_Valve != ON) )
		return FALSE;
    // Count Peripherals ON
    count_peripheral_on = 0;    
    if (PeripheralAct.Peripheral_Types.RotatingTable == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.Cleaner == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.WaterPump == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.Nebulizer_Heater == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.HeaterResistance == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON)
        count_peripheral_on++;
    if (PeripheralAct.Peripheral_Types.Rotating_Valve == ON)
        count_peripheral_on++;
	// Only 1 Peripheral can be ON at the same time
    if (count_peripheral_on > 1)    
		return FALSE;
    
    if ( (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) && (TintingAct.OpenValve_BigHole_state == ON) )
		return FALSE;
    else if ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) && (TintingAct.OpenValve_SmallHole_state == ON) )     
		return FALSE;
//    else if ( (PeripheralAct.Peripheral_Types.RotatingTable == ON) && (TintingAct.RotatingTable_state == ON) )     
//		return FALSE;
            
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
	unsigned long Dos_Temperature;
#ifndef CAR_REFINISHING_MACHINE   
	unsigned long Temperature, RH;
	static unsigned long Process_Neb_Duration;
#endif    
    unsigned char count_periph_on;
    static short int start_timer;

#ifndef CAR_REFINISHING_MACHINE   
  // Check for NEBULIZER/HEATER ERRORS
#ifndef SKIP_FAULT_NEB
    if (TintingHumidifier.Humidifier_Enable == TRUE) {
        if (isFault_Neb_Detection() && (TintingAct.Nebulizer_Heater_state == ON) && (Check_Neb_Error == TRUE) ) {
            NEBULIZER_OFF();
            Check_Neb_Error = FALSE;  
//            NextHumidifier.level = HUMIDIFIER_RUNNING;
//            Humidifier.level = HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR;
            setAlarm(TINTING_WATER_HEATER_OVERCURRENT_THERMAL_ERROR);
        }
        else if (isFault_Neb_Detection() && (TintingAct.Nebulizer_Heater_state == OFF) && (Check_Neb_Error == TRUE) ) {
            NEBULIZER_OFF();
            Check_Neb_Error = FALSE;  
//            NextHumidifier.level = HUMIDIFIER_RUNNING;
//            Humidifier.level = HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR;
            setAlarm(TINTING_WATER_HEATER_OPEN_LOAD_ERROR);
        }
    }    
#endif
    // Check for WATER PUMP = STIRRING DOUBLE_GROUP
#ifndef SKIP_FAULT_PUMP
    if (Double_Group_0 != 64) {
        if (StatusTimer(T_WAIT_AIR_PUMP_TIME) == T_ELAPSED) {
            if (isFault_Pump_Detection() && (DoubleGoup_Stirring_st == ON) ) {
                StopTimer(T_WAIT_AIR_PUMP_TIME); 
                DoubleGoup_Stirring_st = OFF;
             
//                WATER_PUMP_OFF();
                setAlarm(TINTING_DOUBLE_STIRRING_OVERCURRENT_THERMAL_ERROR);
            }
            else if (isFault_Pump_Detection() && (DoubleGoup_Stirring_st == OFF) ) {
                StopTimer(T_WAIT_AIR_PUMP_TIME); 
                DoubleGoup_Stirring_st = OFF;
//                WATER_PUMP_OFF();
                StopTimer(T_WAIT_GENERIC24V_TIME);                 
                setAlarm(TINTING_DOUBLE_STIRRING_OPEN_LOAD_ERROR);    
            }
        }    
    }    
#endif

#endif
    
  // Check for RELE
#ifndef SKIP_FAULT_RELE
    if ( (TintingHumidifier.Temp_Enable == TEMP_ENABLE) && (isAlarmEvaluable() || (Test_rele == ON)) ) {        
        if (StatusTimer(T_WAIT_RELE_TIME) == T_ELAPSED) {
            if (isFault_Rele_Detection() && (TintingAct.HeaterResistance_state == ON) ) {
                StopTimer(T_TEST_RELE);
                StopTimer(T_WAIT_RELE_TIME); 
                RISCALDATORE_OFF();
                setAlarm(TINTING_AIR_HEATER_OVERCURRENT_THERMAL_ERROR);
            }
            else if (isFault_Rele_Detection() && (TintingAct.HeaterResistance_state == OFF) ) {
                StopTimer(T_TEST_RELE);
                StopTimer(T_WAIT_RELE_TIME); 
                RISCALDATORE_OFF();
                setAlarm(TINTING_AIR_HEATER_OPEN_LOAD_ERROR);    
            }
        }
    }    
#endif
  // Check for GENERIC24V --> Spazzola
#ifndef SKIP_FAULT_GENERIC24V 
    if ( ( (TintingClean.Cleaning_Colorant_Mask[1] > 0) || (TintingClean.Cleaning_Colorant_Mask[2] > 0) ) && (Clean_Activation == OFF) && 
           (Punctual_Clean_Act == ON) && isAlarmEvaluable() ) {
        if (StatusTimer(T_WAIT_GENERIC24V_TIME) == T_ELAPSED) {
            if (isFault_Generic24V_Detection() && (TintingAct.Brush_state == ON) ) {
                StopTimer(T_WAIT_GENERIC24V_TIME); 
                TintingAct.Brush_state = OFF;                
//                SPAZZOLA_OFF();
                setAlarm(TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR);
            }
            else if (isFault_Generic24V_Detection() && (TintingAct.Brush_state == OFF) ) {
                StopTimer(T_WAIT_GENERIC24V_TIME); 
                TintingAct.Brush_state = OFF;    
                SPAZZOLA_OFF();
                setAlarm(TINTING_BRUSH_OPEN_LOAD_ERROR);
            }
        }
    }    
#endif
    if ( (StatusTimer(T_WAIT_NEB_ERROR) == T_ELAPSED) && (isAlarmEvaluable()) ) {
        StopTimer(T_WAIT_NEB_ERROR);
        Check_Neb_Error = TRUE;        
    }
    
    switch(Humidifier.level)
    {
        // HUMIDIFIER IDLE
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_IDLE:
            StopTimer(T_HUM_CAP_OPEN_ON);
			StopTimer(T_HUM_CAP_OPEN_PERIOD);
			StopTimer(T_HUM_CAP_CLOSED_ON);
			StopTimer(T_HUM_CAP_CLOSED_PERIOD);
			StopTimer(T_DOS_PERIOD);
			Humidifier_Enable = FALSE;
			Dos_Temperature_Enable = FALSE;
            Humidifier_Count_Disable_Err = 0; 
            Check_Neb_Error = FALSE;
            Check_Neb_Timer = TRUE;
        break;
		// HUMIDIFIER START
		// ------------------------------------------------------------------------------------------------------------        
        case HUMIDIFIER_START:
			StopHumidifier();
            Humidifier_Count_Err = 0;
            Dos_Temperature_Count_Err = 0;            
            start_timer = OFF;
            
            if ( (TintingClean.Cleaning_Colorant_Mask[1] > 0) || (TintingClean.Cleaning_Colorant_Mask[2] > 0) ) {
                StopTimer(T_WAIT_GENERIC24V_TIME); 
                StartTimer(T_WAIT_GENERIC24V_TIME); 
            }    
            
			if ( ((TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_0) && (Humidifier_Count_Disable_Err < HUMIDIFIER_MAX_ERROR_DISABLE))
                                                            ||
                 ((TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_1)) 
															||
                 ((TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) && (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2)) )
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
                TintingAct.Temperature = DOSING_TEMP_PROCESS_DISABLED;
                TintingAct.RH = DOSING_TEMP_PROCESS_DISABLED;
            }            
            if ( (TintingHumidifier.Temp_Enable == TEMP_ENABLE) && (Dos_Temperature_Count_Disable_Err  < DOSING_TEMPERATURE_MAX_ERROR_DISABLE) )
			{
				Dos_Temperature_Enable = TRUE;
				count_dosing_period = 0;
				StartTimer(T_DOS_PERIOD);
                StartTimer(T_WAIT_RELE_TIME);                
				Humidifier.level = HUMIDIFIER_RUNNING;	
//TintingHumidifier.Temp_Period = 1;
			}
            else
			{                
				Dos_Temperature_Enable = FALSE;
                // Symbolic value that means DISABLED
                TintingAct.Dosing_Temperature = DOSING_TEMP_PROCESS_DISABLED;
			}                
			// Check for NEW ommmands receivd
			// ------------------------------------------------------
			// STOP PROCESS command received
            if (isColorCmdStopProcess() == TRUE) 
                StopHumidifier();
			else if (isTintingSetHumidifierHeaterAct() )
			{                
				if (AnalyzeSetupOutputs() == FALSE)
                    setAlarm(HUMIDIFIER_20_PARAM_ERROR);
				else 
                {
                    StopHumidifier();
                    Humidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
			}
			// ------------------------------------------------------
        break;
        // HUMIDIFIER RUNNING
		// ------------------------------------------------------------------------------------------------------------                            
        case HUMIDIFIER_RUNNING:
			// Check for NEW ommmands receivd
			// ------------------------------------------------------
			// STOP PROCESS command received
            if (isColorCmdStopProcess()) 
            {                
				StopHumidifier();
                return;
			}            
			else if (isTintingSetHumidifierHeaterAct() )
			{                
				if (AnalyzeSetupOutputs() == FALSE)
                    setAlarm(HUMIDIFIER_20_PARAM_ERROR);
				else 
                {
                    StopHumidifier();
                    Humidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }    
                return;                
			}
			// ------------------------------------------------------
#ifndef CAR_REFINISHING_MACHINE                
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
					if (TintingHumidifier.Temp_Enable == TEMP_ENABLE)
						StartTimer(T_DOS_PERIOD);
                    
					count_dosing_period = 0;
					count_humidifier_period = 0;
					count_humidifier_period_closed = 0;
					Humidifier.level = HUMIDIFIER_TOO_LOW_WATER_LEVEL;
                    return;
				}
				else
				{	
					// Multiplier = 1000 -> Punp and Nebulizer always ON
					if (TintingHumidifier.Humidifier_Multiplier == 1000)
					{
						TintingAct.Nebulizer_Heater_state = ON;
						if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                            NEBULIZER_ON();
                        // THOR Process
                        else
                        // Nebulizer ON with PWM
                            impostaDuty(TintingHumidifier.Humidifier_PWM);

                            if ( (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) && (Check_Neb_Timer == TRUE) )  
                            {
                                Check_Neb_Timer = FALSE;
                                Check_Neb_Error = FALSE;                                        
                                StartTimer(T_WAIT_NEB_ERROR);
                            }                                    
					}
					// Multiplier = 0 --> Punp and Nebulizer always OFF
					else if (TintingHumidifier.Humidifier_Multiplier == 0)
					{
						TintingAct.Nebulizer_Heater_state = OFF;
                        if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                            NEBULIZER_OFF();                        
                        // THOR Process
                        else
                            // Nebulizer OFF
                            impostaDuty(0);                        
					}
					else
					{	                        
                        //  Manage Humidity Process
                        switch (TintingAct.Autocap_Status)
                        {
                            // Autocap Closed
                            case TINTING_AUTOCAP_CLOSED:
                                if ( (Humidifier.step == STEP_0) || (Humidifier.step == STEP_1))
                                {	
                                    StopTimer(T_HUM_CAP_CLOSED_ON);
                                    StopTimer(T_HUM_CAP_CLOSED_PERIOD);
                                    StartTimer(T_HUM_CAP_CLOSED_ON);
                                    StartTimer(T_HUM_CAP_CLOSED_PERIOD);
                                    count_humidifier_period_closed = 0;
                                    // NEBULIZER (or HEATER Resistance) is ON at the beginning
                                    TintingAct.Nebulizer_Heater_state = ON;
									if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                        NEBULIZER_ON();
                                    // THOR Process
                                    else
                                        // Nebulizer ON with PWM
                                        impostaDuty(TintingHumidifier.Humidifier_PWM);
                                    
                                    if ( (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) && (Check_Neb_Timer == TRUE) )  
                                    {
                                        Check_Neb_Timer = FALSE;
                                        Check_Neb_Error = FALSE;                                        
                                        StartTimer(T_WAIT_NEB_ERROR);
                                    }                                    
                                    Humidifier.step = STEP_2;                                    
                                }
                                else if (Humidifier.step == STEP_2)
                                {
                                    // Check Duration
                                    if (StatusTimer(T_HUM_CAP_CLOSED_ON) == T_ELAPSED) 
                                    {
                                        StopTimer(T_HUM_CAP_CLOSED_ON);
    									TintingAct.Nebulizer_Heater_state = OFF;
										if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                                            NEBULIZER_OFF();                                            
                                        // THOR Process
                                        else 
                                            // Nebulzer OFF
                                            impostaDuty(0);
                                    }
                                    // Check Period
                                    if (StatusTimer(T_HUM_CAP_CLOSED_PERIOD) == T_ELAPSED) 
                                    {
                                        count_humidifier_period_closed++;
                                        StopTimer(T_HUM_CAP_CLOSED_PERIOD);				
                                        StartTimer(T_HUM_CAP_CLOSED_PERIOD);	
//Process_Period = 10;                                        
                                        if (count_humidifier_period_closed >= Process_Period) 
                                        {
                                            count_humidifier_period_closed = 0;    
                                            if (AcquireHumidityTemperature(TintingHumidifier.Humdifier_Type, &Temperature, &RH) == TRUE)
                                            {
                                                TintingAct.Temperature = Temperature;
                                                TintingAct.RH = RH;
                                                HumidifierProcessCalculation(TintingAct.RH, TintingAct.Temperature, 
                                                        &Process_Period, &Process_Neb_Duration);
//    Process_Period = 30;
                                                // 1sec = 5000
                                                Durata[T_HUM_CAP_CLOSED_ON] = Process_Neb_Duration;	

                                                if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                                                    NEBULIZER_ON();
                                                else 
                                                    // Nebulizer ON with PWM
                                                    impostaDuty(TintingHumidifier.Humidifier_PWM);

                                                StopTimer(T_HUM_CAP_CLOSED_ON);
                                                StartTimer(T_HUM_CAP_CLOSED_ON);
                                                // Only NEBULIZER is ON at the beginning
                                                TintingAct.Nebulizer_Heater_state = ON;
                                            }                                         
                                            else
                                            {	
    //                                          StopTimer(T_HUM_CAP_CLOSED_PERIOD);
                                                Humidifier_Count_Err++;
                                                Humidifier_Count_Disable_Err++;
                                                TintingAct.Nebulizer_Heater_state = OFF;
                                                if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                                                    NEBULIZER_OFF();                                            
                                                // THOR Process
                                                else 
                                                    // Nebulzer OFF
                                                    impostaDuty(0);
                                                
                                                if (Humidifier_Count_Err >= HUMIDIFIER_MAX_ERROR)
                                                {
                                                    // Symbolic value that means DISABLED
                                                    TintingAct.Temperature = DOSING_TEMP_PROCESS_DISABLED;
                                                    TintingAct.RH = DOSING_TEMP_PROCESS_DISABLED;                
                                                    Humidifier_Enable = FALSE;
                                                }
                                                setAlarm(RH_ERROR);
                                            }
                                        }										
                                    }
                                }    
                            break;

                            // Autocap Open
                            case TINTING_AUTOCAP_OPEN:
                                // Period = 0 -> Nebulizer (or Heater) always ON
                                if (TintingHumidifier.AutocapOpen_Period == 0)
                                {
                                    TintingAct.Nebulizer_Heater_state = ON;
                                    if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                        NEBULIZER_ON();
                                    // THOR Process
                                    else
                                        // Nebulizer ON with PWM
                                        impostaDuty(TintingHumidifier.Humidifier_PWM);

                                    if ( (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) && (Check_Neb_Timer == TRUE) )  
                                    {
                                        Check_Neb_Timer = FALSE;
                                        Check_Neb_Error = FALSE;                                        
                                        StartTimer(T_WAIT_NEB_ERROR);
                                    }                                    
                                }
                                // Duration = 0 AND Period != 0 -> Nebulizer (or Heater) always OFF
                                else if (TintingHumidifier.AutocapOpen_Duration == 0)
                                {
                                    TintingAct.Nebulizer_Heater_state = OFF;
                                    if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                        NEBULIZER_OFF();
                                    // THOR Process
                                    else
                                        // Nebulizer OFF
                                        impostaDuty(0);                                    
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
                                        if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                            NEBULIZER_ON();                                        
                                        // THOR Process
                                        else
                                            // Nebulizer ON with PWM
                                            impostaDuty(TintingHumidifier.Humidifier_PWM);
                                        
                                        if ( (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) && (Check_Neb_Timer == TRUE) )  
                                        {
                                            Check_Neb_Timer = FALSE;
                                            Check_Neb_Error = FALSE;                                        
                                            StartTimer(T_WAIT_NEB_ERROR);
                                        }                                                                            
                                        Humidifier.step = STEP_1;
                                    }
                                    else if (Humidifier.step == STEP_1)
                                    {
                                        // Check Duration
                                        if (StatusTimer(T_HUM_CAP_OPEN_ON) == T_ELAPSED)
                                        {
                                            StopTimer(T_HUM_CAP_OPEN_ON);
                                            TintingAct.Nebulizer_Heater_state = OFF;
                                            if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                                NEBULIZER_OFF();
                                            // THOR Process
                                            else
                                                // Nebulizer OFF
                                                impostaDuty(0);                                            
                                        }
                                        // Check Period
                                        if (StatusTimer(T_HUM_CAP_OPEN_PERIOD) == T_ELAPSED) 
                                        {
                                            count_humidifier_period++;
                                            StopTimer(T_HUM_CAP_OPEN_PERIOD);				
                                            StartTimer(T_HUM_CAP_OPEN_PERIOD);	
                                            if (count_humidifier_period == TintingHumidifier.AutocapOpen_Period) 
                                            {
                                                StartTimer(T_HUM_CAP_OPEN_ON);
                                                count_humidifier_period = 0;
                                                TintingAct.Nebulizer_Heater_state = ON;
                                                if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                                                    NEBULIZER_ON();
                                                // THOR Process
                                                else
                                                    // Nebulizer ON with PWM
                                                    impostaDuty(TintingHumidifier.Humidifier_PWM);                                                                                                
                                            }
                                        }										
                                    }							
                                }
                            break;

                            // Autocap Error				
                            case TINTING_AUTOCAP_ERROR:
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
#endif            
			// Dosing Temperature process
			if ( (Dos_Temperature_Enable == TRUE) && isAlarmEvaluable() )
			{
				if (StatusTimer(T_DOS_PERIOD) == T_ELAPSED) 
				{
					count_dosing_period++;
					StopTimer(T_DOS_PERIOD);			
					StartTimer(T_DOS_PERIOD);			
//TintingHumidifier.Temp_Period = 10;                    
					if (count_dosing_period >= TintingHumidifier.Temp_Period) 
					{
						count_dosing_period = 0;                        
						if (AcquireTemperature(TintingHumidifier.Temp_Type, &Dos_Temperature) == TRUE)
						{
                            Dos_Temperature_Count_Err = 0;
                            TintingAct.Dosing_Temperature = Dos_Temperature;
                            if ( (TintingAct.HeaterResistance_state == ON) && ((Table_Motors == ON) || (Pump_Valve_Motors == ON) || (autocapAct.autocapFlags.running == TRUE) || (isBasesMotorCircuitsRunning() == TRUE) ) ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }                                
                            else if ( (TintingAct.HeaterResistance_state == ON) && (TintingAct.Dosing_Temperature/10) >= (unsigned long)(TintingHumidifier.Heater_Temp + TintingHumidifier.Heater_Hysteresis) ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                                                             
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }
                            else if ( (TintingAct.HeaterResistance_state == OFF) && ( (TintingAct.Dosing_Temperature/10) <= (unsigned long)(TintingHumidifier.Heater_Temp - TintingHumidifier.Heater_Hysteresis) ) &&
                                      (Table_Motors == OFF) && (Pump_Valve_Motors == OFF) && (autocapAct.autocapFlags.running == FALSE) && (isBasesMotorCircuitsRunning() == FALSE) ) {
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
                                TintingAct.Dosing_Temperature = DOSING_TEMP_PROCESS_DISABLED;                                
                                Dos_Temperature_Enable = FALSE;
                                setAlarm(TEMPERATURE_ERROR);                                
                            }
                        }
					}
                    else {
                        if ( (TintingAct.HeaterResistance_state == ON) && ((Table_Motors == ON) || (Pump_Valve_Motors == ON) || (autocapAct.autocapFlags.running == TRUE) || (isBasesMotorCircuitsRunning() == TRUE) ) ) {
                            StopTimer(T_WAIT_RELE_TIME);                                
                            TintingAct.HeaterResistance_state = OFF;
                            RISCALDATORE_OFF();
                        }                                
                    }                                
				}					
			}	
		break;		
		// PERIPHERAL OUTPUT MANAGER
		// ------------------------------------------------------------------------------------------------------------
        case HUMIDIFIER_SETUP_OUTPUT:
			// A 'TintingAct.Output_Act' == OUTPUT_OFF' is arrived
            if (isTintingSetHumidifierHeaterAct() )
			{
				if (AnalyzeSetupOutputs() == FALSE)
                    setAlarm(HUMIDIFIER_20_PARAM_ERROR);
				else 
                {
                    Humidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }
			}
			// STOP PROCESS command received
            if (isColorCmdStopProcess() ) {
				StopHumidifier();
                Humidifier.level = HUMIDIFIER_START;
                return;                
            }
            // RESET command received
            else if (isColorCmdIntr() ) {
				StopHumidifier();
                Humidifier.level = HUMIDIFIER_START;
                return;                
            }
            // Brush
            if (PeripheralAct.Peripheral_Types.Cleaner == ON) {          
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    StopTimer(T_WAIT_GENERIC24V_TIME);                                
                    StartTimer(T_WAIT_GENERIC24V_TIME);                                                                                                                     
                    TintingAct.Brush_state = ON;
                    SPAZZOLA_ON();
                }    
                else {
                    StopTimer(T_WAIT_GENERIC24V_TIME);                                
                    StartTimer(T_WAIT_GENERIC24V_TIME);                                                                                                                     
                    TintingAct.Brush_state = OFF;
                    SPAZZOLA_OFF();
                }    
            }
            // Water Pump = Stirring   
            else if (PeripheralAct.Peripheral_Types.WaterPump == ON) {        
                if ( (TintingAct.Output_Act == OUTPUT_ON) && (TintingAct.WaterPump_state == OFF) ) {
                    TintingAct.WaterPump_state = ON;
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    //StartTimer(T_WAIT_STIRRING_ON);                                        
                    WATER_PUMP_ON();
                }    
                else if ( (TintingAct.Output_Act == OUTPUT_OFF) && (TintingAct.WaterPump_state == ON) ) {
                    TintingAct.WaterPump_state = OFF;
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    //StopTimer(T_WAIT_STIRRING_ON);                                        
                    WATER_PUMP_OFF();
                }    
            }
#ifndef CAR_REFINISHING_MACHINE            
            // Nebulizer or Heater
            else if (PeripheralAct.Peripheral_Types.Nebulizer_Heater == ON) { 
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    TintingAct.Nebulizer_Heater_state = ON;
					if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2)
                        NEBULIZER_ON();    
                    // THOR Process
                    else
                    // Nebulizer ON with PWM
                        impostaDuty(TintingHumidifier.Humidifier_PWM);
                    
                    if ( (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) && (Check_Neb_Timer == TRUE) ) {
                        Check_Neb_Timer = FALSE;
                        Check_Neb_Error = FALSE;                                        
                        StartTimer(T_WAIT_NEB_ERROR);
                    }                                                                            
                }    
                else {
                    TintingAct.Nebulizer_Heater_state = OFF;
                    if (TintingHumidifier.Humdifier_Type != HUMIDIFIER_TYPE_2) 
                        NEBULIZER_OFF();                        
                    // THOR Process
                    else
                    // Nebulizer OFF
                        impostaDuty(0);                       
                    
                    if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) {
                        Check_Neb_Timer = TRUE;  
                        Check_Neb_Error = FALSE;                                                                
                    }                                                                                                    
                }    
            }
#endif            
            // Heater Resistance    
            else if (PeripheralAct.Peripheral_Types.HeaterResistance == ON) {
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    StopTimer(T_WAIT_RELE_TIME);                                
                    StartTimer(T_WAIT_RELE_TIME);                                                                                                 
                    TintingAct.HeaterResistance_state = ON;
                    RISCALDATORE_ON();
                }    
                else {
                    StopTimer(T_WAIT_RELE_TIME);                                
                    StartTimer(T_WAIT_RELE_TIME);                                                                                                 
                    TintingAct.HeaterResistance_state = OFF;
                    RISCALDATORE_OFF();
                }    
            }
            // No peripheral selected
            PeripheralAct.Peripheral_Types.bytePeripheral = 0;
            // Count Peripherals ON
            count_periph_on = 0;
            if (TintingAct.Brush_state == ON)
                count_periph_on++;
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
        // HUMIDIFIER TOO LOW WATER LEVEL
		// ------------------------------------------------------------------------------------------------------------        
#ifndef CAR_REFINISHING_MACHINE
        case HUMIDIFIER_TOO_LOW_WATER_LEVEL:
			// Check for NEW ommmands received
			// ------------------------------------------------------
			// STOP PROCESS command received
			if(isColorCmdStopProcess())
			{
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				StopHumidifier();
				Humidifier.level = HUMIDIFIER_START;
                return;
			}
			else if (isTintingSetHumidifierHeaterAct() )
			{                
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				if (AnalyzeSetupOutputs() == FALSE)
                    setAlarm(HUMIDIFIER_20_PARAM_ERROR);
				else 
                {
                    StopHumidifier();
                    Humidifier.level = HUMIDIFIER_SETUP_OUTPUT;
                }
                return;                
			}            
			
            if (getWaterLevel() == ON) {
				// Gestione Lampeggio LED
				StopTimer(T_DOS_PERIOD);
				count_dosing_period = 0;
				Humidifier.level = HUMIDIFIER_START;
                return;                
			}
			// Manage Dosing Temperature process if activated
			if ( (Dos_Temperature_Enable == TEMP_ENABLE) && isAlarmEvaluable() )
			{
				if (StatusTimer(T_DOS_PERIOD) == T_ELAPSED) 
				{
					count_dosing_period++;
					StopTimer(T_DOS_PERIOD);			
					StartTimer(T_DOS_PERIOD);							
					if (count_dosing_period == TintingHumidifier.Temp_Period) 
					{
						count_dosing_period = 0;
						if (AcquireTemperature(TintingHumidifier.Temp_Type, &Dos_Temperature) == TRUE)
						{
                            Dos_Temperature_Count_Err = 0;
                            TintingAct.Dosing_Temperature = Dos_Temperature;
                            if ( (TintingAct.HeaterResistance_state == ON) && ((Table_Motors == ON) || (Pump_Valve_Motors == ON) || (autocapAct.autocapFlags.running == TRUE) || (isBasesMotorCircuitsRunning() == TRUE) ) ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }                                
                            else if ( (TintingAct.HeaterResistance_state == ON) && (TintingAct.Dosing_Temperature/10) >= (unsigned long)(TintingHumidifier.Heater_Temp + TintingHumidifier.Heater_Hysteresis) ) {
                                StopTimer(T_WAIT_RELE_TIME);                                
                                StartTimer(T_WAIT_RELE_TIME);                                                                             
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();
                            }
                            else if ( (TintingAct.HeaterResistance_state == OFF) && ( (TintingAct.Dosing_Temperature/10) <= (unsigned long)(TintingHumidifier.Heater_Temp - TintingHumidifier.Heater_Hysteresis) ) &&
                                      (Table_Motors == OFF) && (Pump_Valve_Motors == OFF) && (autocapAct.autocapFlags.running == FALSE) && (isBasesMotorCircuitsRunning() == FALSE) ) {
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
                                TintingAct.Dosing_Temperature = DOSING_TEMP_PROCESS_DISABLED;                                
                                Dos_Temperature_Enable = FALSE;
                                setAlarm(TEMPERATURE_ERROR);                                
                            }                                                        
						}
					}
                    else {
                        if ( (TintingAct.HeaterResistance_state == ON) && ((Table_Motors == ON) || (Pump_Valve_Motors == ON) || (autocapAct.autocapFlags.running == TRUE) || (isBasesMotorCircuitsRunning() == TRUE) ) ) {
                            StopTimer(T_WAIT_RELE_TIME);                                
                            TintingAct.HeaterResistance_state = OFF;
                            RISCALDATORE_OFF();
                        }                                
                    }                                                    
				}					
			}
        break;
#endif        
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
//TC72_Temperature = 200;                
//*Temp = TC72_Temperature;
//return TRUE;            
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
                                                                              * 
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
            *Temp = DOSING_TEMP_PROCESS_DISABLED;
            *Humidity = DOSING_TEMP_PROCESS_DISABLED;        
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
//	float KT1, KH1, KT2, KH2, KBoostT, KBoostH, KBoostTH, KRT, KRH, Temp, RH_Humidity, Knormalizz, Period_calc;
	
    // NO SENSOR - Process Humidifier 1.0
    if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_1)
    {
        // 1 = 1sec
        *Period = TintingHumidifier.Humidifier_Period;  
        // 1 = 2msec
        *Neb_Duration = TintingHumidifier.Humidifier_Multiplier * 5000;
    }
    // NO SENSOR - THOR process
    else if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2)
    {
        // 1 = 1sec
        *Period = TintingHumidifier.Humidifier_Period;  
        // 1 = 2msec
        *Neb_Duration = TintingHumidifier.Humidifier_Multiplier * 5000;
    }     
    // SENSOR SHT31 - Process Humidifier 2.0
    else
    {
/*        
        Temp = (float)Temperature/10;
        RH_Humidity = (float)RH/1000;
//	Temp = (float)20;
//	RH_Humidity = (float)0.5;

        // Normalization factor betwee old system and NEW one x conversion factor between sec to 2msec unit (= 0.007692 * 5000)
        Knormalizz = 3.846;

        // KT1 = variabile di temperatura per definizione del "Period"
        // Calcolo KT1
        if (Temp < 10)
            KT1 = 0.75;
        else if (Temp > 30)
            KT1 = 0.25;
        else
            KT1 = 0.5 + (20 - Temp) * 0.025;

        // KH1 = variabile di umidita'† per definizione del "Period"
        // Calcolo KH1
        if (RH_Humidity < 0.4)
            KH1 = 0.25;
        else if (RH_Humidity > 0.8)
            KH1 = 0.75;
        else
            KH1 = 0.5 -(0.6 - RH_Humidity) * 5 / 4;

        // "Period = tempo all'interno del quale si calcola il tempo di funzionamento della Pompa e del Nebulizzatore dell'acuqa della bottiglia
        // Calcolo "Period"
        *Period = TintingHumidifier.Humidifier_Period * (KT1 + KH1);
        Period_calc = (float)T0.8AintingHumidifier.Humidifier_Period * (KT1 + KH1);

        // KT2 = variabile di temperatura per definizione di "Pump_Duration"
        // Calcolo KT2
        if (Temp < 10)
            KT2 = 0.5;
        else if (Temp > 30)
            KT2 = 1.5;
        else
            KT2 = 0.5 + (Temp - 10) / 20;

        // KH2 = variabile di umidit√† per definizione del "Pump_Duration"
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

        // KBoostH = variabile di umidit√† per aumentare il "Period" in un determinato range di H
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

        // KRH = variabile di umidit√† per aumentare il "Neb_Duration" in un determinato range di H
        // Calcolo KRH 
        if (RH_Humidity < 0.2)
            KRH = 0.45;
        else if (RH_Humidity > 0.5)
            KRH = 0.0;
        else
            KRH = (0.5 - RH_Humidity) * 1.5;

        *Neb_Duration = Period_calc * KT2 * KH2 * KBoostTH * Knormalizz;
        if ((*Neb_Duration / 5000) > *Period)
            *Neb_Duration = *Period * 5000;

        // "Neb_Duration" = tempo durante il quale il Nebulizzatore o la Resistenza dell'acqua della bottiglia rimane acceso
        // Calcolo "Neb_Duration"
        *Neb_Duration = *Neb_Duration * (0.1 + KRT + KRH);
    */
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
