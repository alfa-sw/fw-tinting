/* 
 * File:   stepperParameters.h
 * Author: michele abelli
 *
 * Created on 12 marzo 2018, 12.34
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
// MIXER MOTOR        
#define RESOLUTION_MIXER     4 // 1/16 steps
#if (RESOLUTION_MIXER == 1)
    #define CORRECTION_MIXER_STEP_RES 1
#elif (RESOLUTION_MIXER == 2)
    #define CORRECTION_MIXER_STEP_RES 2 
#elif (RESOLUTION_MIXER == 3)
    #define CORRECTION_MIXER_STEP_RES 4 
#elif (RESOLUTION_MIXER == 4)
    #define CORRECTION_MIXER_STEP_RES 8 
#elif (RESOLUTION_MIXER == 5)
    #define CORRECTION_MIXER_STEP_RES 16 
#elif (RESOLUTION_MIXER == 6)
    #define CORRECTION_MIXER_STEP_RES 32 
#elif (RESOLUTION_MIXER == 7)
    #define CORRECTION_MIXER_STEP_RES 64 
#elif (RESOLUTION_MIXER == 8)
    #define CORRECTION_MIXER_STEP_RES 128 
#endif 
// Phase Current (RMS) during ramp movement (= A x 10) 
#define RAMP_PHASE_CURRENT_MIXER 55 // 5.5 A
// Phase Current (RMS) during constans speed movement (= A x 10) 
// Passare in input valori  correnti dA (picco).  Questo dato verrà convertito per settare il corretto registro del driver
#define PHASE_CURRENT_MIXER   55 // 5.5 A  
#define HOLDING_CURRENT_MIXER_ENGAGE 12 // 1.2 A      
#define HOLDING_CURRENT_MIXER_MOVING 2 // 0.2 A      
// Acceleration (step/sec^2) during acceleration ramp
#define ACC_RATE_MIXER       1000 // 1000 step /sec^2
// Deceleration (step/sec^2) during deceleration ramp
#define DEC_RATE_MIXER       5000 // 5000 step /sec^2  
//------------------------------------------------------------------------------    
// DOOR MOTOR        
#define RESOLUTION_DOOR    4 // 1/16 steps
#if (RESOLUTION_DOOR == 1)
    #define CORRECTION_DOOR_STEP_RES 1
#elif (RESOLUTION_DOOR == 2)
    #define CORRECTION_DOOR_STEP_RES 2 
#elif (RESOLUTION_DOOR == 3)
    #define CORRECTION_DOOR_STEP_RES 4 
#elif (RESOLUTION_DOOR == 4)
    #define CORRECTION_DOOR_STEP_RES 8 
#elif (RESOLUTION_DOOR == 5)
    #define CORRECTION_DOOR_STEP_RES 16 
#elif (RESOLUTION_DOOR == 6)
    #define CORRECTION_DOOR_STEP_RES 32 
#elif (RESOLUTION_DOOR == 7)
    #define CORRECTION_DOOR_STEP_RES 64 
#elif (RESOLUTION_DOOR == 8)
    #define CORRECTION_DOOR_STEP_RES 128 
#endif 

// Phase Current (RMS) during ramp movement (= A x 10) 
#define RAMP_PHASE_CURRENT_DOOR 15 // 1.5 A
// Phase Current (RMS) during constans speed movement (= A x 10) 
// Passare in input valori  correnti dA (picco).  Questo dato verrà convertito per settare il corretto registro del driver
#define PHASE_CURRENT_DOOR   15 // 1.5 A  
#define HOLDING_CURRENT_DOOR_ENGAGE 1 // 0.1 A      
//#define HOLDING_CURRENT_DOOR_MOVING 5 // 0.5 A      
#define HOLDING_CURRENT_DOOR_MOVING 10 // 0.5 A      
// Acceleration (step/sec^2) during acceleration ramp
#define ACC_RATE_DOOR       5000 // 30000 step /sec^2
// Deceleration (step/sec^2) during deceleration ramp
#define DEC_RATE_DOOR       5000 // 30000 step /sec^2  
//------------------------------------------------------------------------------    
// AUTOCAP MOTOR  
#define RESOLUTION_AUTOCAP    4 // 1/16 steps
#if (RESOLUTION_AUTOCAP == 1)
    #define CORRECTION_AUTOCAP_STEP_RES 1
#elif (RESOLUTION_AUTOCAP == 2)
    #define CORRECTION_AUTOCAP_STEP_RES 2 
#elif (RESOLUTION_AUTOCAP == 3)
    #define CORRECTION_AUTOCAP_STEP_RES 4 
#elif (RESOLUTION_AUTOCAP == 4)
    #define CORRECTION_AUTOCAP_STEP_RES 8 
#elif (RESOLUTION_AUTOCAP == 5)
    #define CORRECTION_AUTOCAP_STEP_RES 16 
#elif (RESOLUTION_AUTOCAP == 6)
    #define CORRECTION_AUTOCAP_STEP_RES 32 
#elif (RESOLUTION_AUTOCAP == 7)
    #define CORRECTION_AUTOCAP_STEP_RES 64 
#elif (RESOLUTION_AUTOCAP == 8)
    #define CORRECTION_AUTOCAP_STEP_RES 128 
#endif 
// Phase Current (RMS) during ramp movement (= A x 10) 
#define RAMP_PHASE_CURRENT_AUTOCAP 14 // 1.4 A
    // Phase Current (RMS) during constans speed movement (= A x 10) 
// Passare in input valori  correnti dA (picco).  Questo dato verrà convertito per settare il corretto registro del driver
#define PHASE_CURRENT_AUTOCAP   14 // 1.4 A  

#define HOLDING_CURRENT_AUTOCAP_ENGAGE 1 // 0.1 A      
#define HOLDING_CURRENT_AUTOCAP_MOVING 1 // 0.1 A      
// Acceleration (step/sec^2) during acceleration ramp
#define ACC_RATE_AUTOCAP       5000 // 1000 step /sec^2
// Deceleration (step/sec^2) during deceleration ramp
#define DEC_RATE_AUTOCAP       5000 // 5000 step /sec^2  
//------------------------------------------------------------------------------    
// Acceleration / Deceleration values:
// Min : 1000 Step/s^2  
// Max: 59000 Step/s^2
    
// Step:  1000 Step/s^2
// Maximum Mixer Acceleration and Deceleration (step/sec^2)    
#define MAX_ACC_RATE_MIXER   59000 // 50000 step /sec^2
#define MAX_DEC_RATE_MIXER   59000 // 50000 step /sec^2
// Alarms Enable mask (0 = Disable, 1 = Enabled)
#define ALARMS_MIXER         0xFF  // Enabled
#define ALARMS_DOOR          0xFF  // Enabled  
#define ALARMS_AUTOCAP       0xFF  // Enabled      
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
// MIN and MAX limit
#define MAXIMUM_SPEED   1200 // (rpm))

#ifdef	__cplusplus
}
#endif

#endif	/* STEPPERPARAMETERS_H */

