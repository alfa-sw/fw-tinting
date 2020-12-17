/* 
 * File:   stepperParameters.h
 * Author: mballerini
 *
 * Created on 12 novembre 2018, 14.56
 */

#ifndef STEPPERPARAMETERS_H
#define	STEPPERPARAMETERS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Motor Parameters
// Speed Limits
// Min : 0 Step/s
// Max: 1200 Step/s
// Step:  10 

// Motor Resolution (1/1: 0, ½: 1, ¼: 2, 1/8: 3, 1/16: 4, 1/32: 5, 1/64: 6, 1/128: 7, 1/256: 8)
// Maximim admitted Resolution: 1/16
// -----------------------------------------------------------------------------
#define RESOLUTION_VALVE    4 // 1/16 steps
#if (RESOLUTION_VALVE == 1)
    #define CORRECTION_VALVE_STEP_RES 1
#elif (RESOLUTION_VALVE == 2)
    #define CORRECTION_VALVE_STEP_RES 2 
#elif (RESOLUTION_VALVE == 3)
    #define CORRECTION_VALVE_STEP_RES 4 
#elif (RESOLUTION_VALVE == 4)
    #define CORRECTION_VALVE_STEP_RES 8 
#elif (RESOLUTION_VALVE == 5)
    #define CORRECTION_VALVE_STEP_RES 16 
#elif (RESOLUTION_VALVE == 6)
    #define CORRECTION_VALVE_STEP_RES 32 
#elif (RESOLUTION_VALVE == 7)
    #define CORRECTION_VALVE_STEP_RES 64 
#elif (RESOLUTION_VALVE == 8)
    #define CORRECTION_VALVE_STEP_RES 128 
#endif 
// -----------------------------------------------------------------------------
#define RESOLUTION_TABLE    4 // 1/16 steps  
#if (RESOLUTION_TABLE == 1)
    #define CORRECTION_TABLE_STEP_RES 1
#elif (RESOLUTION_TABLE == 2)
    #define CORRECTION_TABLE_STEP_RES 2 
#elif (RESOLUTION_TABLE == 3)
    #define CORRECTION_TABLE_STEP_RES 4 
#elif (RESOLUTION_TABLE == 4)
    #define CORRECTION_TABLE_STEP_RES 8 
#elif (RESOLUTION_TABLE == 5)
    #define CORRECTION_TABLE_STEP_RES 16 
#elif (RESOLUTION_TABLE == 6)
    #define CORRECTION_TABLE_STEP_RES 32 
#elif (RESOLUTION_TABLE == 7)
    #define CORRECTION_TABLE_STEP_RES 64 
#elif (RESOLUTION_TABLE == 8)
    #define CORRECTION_TABLE_STEP_RES 128 
#endif 
// -----------------------------------------------------------------------------
#define RESOLUTION_PUMP     4 // 1/16 steps
#if (RESOLUTION_PUMP == 1)
    #define CORRECTION_PUMP_STEP_RES 1
#elif (RESOLUTION_PUMP == 2)
    #define CORRECTION_PUMP_STEP_RES 2 
#elif (RESOLUTION_PUMP == 3)
    #define CORRECTION_PUMP_STEP_RES 4 
#elif (RESOLUTION_PUMP == 4)
    #define CORRECTION_PUMP_STEP_RES 8 
#elif (RESOLUTION_PUMP == 5)
    #define CORRECTION_PUMP_STEP_RES 16 
#elif (RESOLUTION_PUMP == 6)
    #define CORRECTION_PUMP_STEP_RES 32 
#elif (RESOLUTION_PUMP == 7)
    #define CORRECTION_PUMP_STEP_RES 64 
#elif (RESOLUTION_PUMP == 8)
    #define CORRECTION_PUMP_STEP_RES 128 
#endif 
        
// Phase Current (RMS) during ramp movement (= A x 10) 
//#define RAMP_PHASE_CURRENT_TABLE    45 // 4.5 A  
#define RAMP_PHASE_CURRENT_TABLE    64 // 6.4 A  
#define RAMP_PHASE_CURRENT_PUMP     45 // 4.5 A
#define RAMP_PHASE_CURRENT_VALVE    45 // 4.5 A
#define HOMING_RAMP_PHASE_CURRENT_VALVE  45 // 4.5 A      
//#define HOMING_RAMP_PHASE_CURRENT_VALVE  20 // 2.0 A      
// Phase Current (RMS) during constans speed movement (= A x 10) 
// Passare in input valori  correnti dA (picco).  Questo dato verrà convertito per settare il corretto registro del driver
//#define PHASE_CURRENT_TABLE  45 // 4.5 A  
#define PHASE_CURRENT_TABLE  64 // 6.4 A  
#define PHASE_CURRENT_PUMP   45 // 4.5 A  
//#define PHASE_CURRENT_VALVE  45 // 4.5 A  
//#define HOMING_PHASE_CURRENT_VALVE  45 // 4.5 A 
#define PHASE_CURRENT_VALVE  60 // 6.0 A  
#define HOMING_PHASE_CURRENT_VALVE  60 // 6.0 A      
//#define HOMING_PHASE_CURRENT_VALVE  20 // 2.0 A  
// Holding Current (RMS) (= A x 10) 
#define HOLDING_CURRENT_TABLE 12 // 1.2 A  
#define HOLDING_CURRENT_TABLE_FINAL 45 // 4.5 A  
//#define HOLDING_CURRENT_TABLE_FINAL 64 // 6.4 A  
#define HOLDING_CURRENT_TABLE_PUMP_ENGAGE 12 // 1.2 A      
#define HOLDING_CURRENT_TABLE_PUMP_MOVING 2 // 0.2 A      
#define HOLDING_CURRENT_PUMP  0 // 0.0 A
#define HOLDING_CURRENT_VALVE 3 // 0.3 A
#define HOLDING_CURRENT_VALVE_DOSING 20 // 2.0 A

// Acceleration (step/sec^2) during acceleration ramp
#define ACC_RATE_TABLE      1000 // 5000 step /sec^2  
//#define ACC_RATE_TABLE      5000 // 5000 step /sec^2  
#define ACC_RATE_PUMP       30000 // 30000 step /sec^2
#define ACC_RATE_VALVE      30000 // 30000 step /sec^2
// Deceleration (step/sec^2) during deceleration ramp
#define DEC_RATE_TABLE      1000 // 5000 step /sec^2   
//#define DEC_RATE_TABLE      5000 // 5000 step /sec^2   
#define DEC_RATE_PUMP       30000 // 30000 step /sec^2  
#define DEC_RATE_VALVE      30000 // 30000 step /sec^2 
// Acceleration / Deceleration values:
// Min : 1000 Step/s^2  
// Max: 59000 Step/s^2
    
// Step:  1000 Step/s^2
// Maximum Pump Acceleration and Deceleration (step/sec^2)    
#define MAX_ACC_RATE_PUMP   59000 // 50000 step /sec^2
#define MAX_DEC_RATE_PUMP   59000 // 50000 step /sec^2
// Maximum Valve Acceleration and Deceleration (step/sec^2)
#define MAX_ACC_RATE_VALVE   59000 // 50000 step /sec^2
#define MAX_DEC_RATE_VALVE   59000 // 50000 step /sec^2    
// Alarms Enable mask (0 = Disable, 1 = Enabled)
#define ALARMS_TABLE        0xFF  // Enabled  
#define ALARMS_PUMP         0xFF  // Enabled
#define ALARMS_VALVE        0xFF  // Enabled
// Alarms Bit Mask types
/*
#define OVER_CURRENT_DETECTION  0x01 // bit0 - OCD flag: bit13
#define THERMAL_SHUTDOWN        0x02 // bit1 - TH_STATUS 10,11: bit11-12
#define THERMAL_WARNING         0x04 // bit2 - TH_STATUS 01: bit11-12
#define UNDER_VOLTAGE_LOCK_OUT  0x08 // bit3 - UVLO flag: bit9
#define STALL_DETECTION         0x10 // bit4
#define UNDER_VOLTAGE_LOCK_OUT_ADC  0x20 // bit4 - UVLO_ADC flag: bit8
*/
#define OVER_CURRENT_DETECTION  0x2000   // OCD flag: bit13
#define UNDER_VOLTAGE_LOCK_OUT  0x200    // UVLO flag: bit9
#define UNDER_VOLTAGE_LOCK_OUT_ADC 0x400 // UVLO_ADC flag: bit10    
#define THERMAL_ERROR           0x1800   // TH_STATUS flags: bit11-12
#define THERMAL_SHUTDOWN_BRIDGE 0x02     // TH_STATUS 10
#define THERMAL_SHUTDOWN_DEVICE 0x03     // TH_STATUS 11
#define THERMAL_WARNING         0x04     // TH_STATUS 01
#define MOT_STS                 0x60     // MOT_STS flags: bit5, bit6    
// -----------------------------------------------------------------------------
// Types of Algorithm
#define ALG_SINGLE_STROKE          (0)
#define ALG_DOUBLE_STROKE          (1)
#define ALG_SYMMETRIC_CONTINUOUS   (2)
#define ALG_ASYMMETRIC_CONTINUOUS  (3)
#define HIGH_RES_STROKE  		   (4)
// -----------------------------------------------------------------------------
// MIN and MAX limit
#define MAXIMUM_SPEED   1200 // (rpm))


#ifdef	__cplusplus
}
#endif

#endif	/* STEPPERPARAMETERS_H */

