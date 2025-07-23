/*Header for defining a Sensor with parameter of measurment range.
The class member functions calculate the necessary variables for calculating the analog current value from digital sensor value. 
With the analog current value the measured physical parameters can be converted. These calculations can be made in the respective firmware*/
//Created by Csaba Freiberger 09.2022
#ifndef SensorClasses_h
#define SensorClasses_h
#include <stdlib.h>
#include "SerialMon.h"

/**
 * @brief Enum for the output signal type
 * @param currentSignal     current signal type
 * @param voltageSignal     voltage signal type
 * @param digitalSignal     digital signal type
 * 
 */
class OutputSignalType
{
public:
    enum signalType{
        currentSignal,
        voltageSignal,
        digitalSignal
    };
};

/**
 * @brief Struct for the measuring range of the sensor
 * @param minOutputValue    min measuring value
 * @param maxOutputValue    max measuring value
 * @param measuringOffset   offset of the measuring value
 * 
 */
struct MeasuringRange
{
    float minOutputValue;
    float maxOutputValue;
    float measuringOffset;
};

/**
 * @brief Struct for the signal range of the sensor. 
 * This can be a current signal range or a voltage signal range
 * @param minSignalValue    min signal value
 * @param maxSignalValue    max signal value
 * @param signalOffset      offset of the signal value
 * 
 */
struct SignalRange{
    float minSignalValue;
    float maxSignalValue;
    float signalOffset;
};


/**
 * @brief Class for construct sensor type
 * @param encoder       rotatory sensor
 * @param lvdt          sensor where the sensor length is not the KP length like for LVDT
 * @param cablePull     sensor where the sensor length is the same as the KP length like for a cable-pull sensor
 * @param capacitive    capacitive sensor, which can be used for angular motion
 * 
 */
class SensorType
{
public:
   enum sensorType{
        encoder,
        lvdt,
        cablePull,
        capacitive
   };
};



/**
 * @brief Construct a new Sensor Class object. \n 
 * The Sensor Types are: encoder for rotatory sensors, lvdt for sensors where the sensor length is not the KP length like for LVDT,
 * and cable-pull for sensors where the sensor length is the same as the KP length like for a cable-pull sensor
 */
class SensorClass : public OutputSignalType, public SensorType{
    private:

        /**
         * @brief calculation of the measurement range of the sensor
         * 
         */
        void calcRange();

        /**
         * @brief Calculate the resolution of the sensor ( measured unit/signal unit )
         * 
         */
        void calcResolution();

        /**
         * @brief Calc function to determine the output signal value (mA or V) if the measured value is 0 (measured unit)
         * 
         */
        void calcZeroValue();

    public:

        sensorType mySensorType;                //enum for sensor type
        signalType mySignalType;                 //enum for signal type
        bool measuringMinIsSignalMin = true;   //bool for mapping correct measuring value to signal value

        /**
         * @brief Construct a new Sensor Class object with sensor type
         * 
         * @param _sensorType               enum sensorType. Set the Sensor Type: encoder, lvdt or cable-pull
         * @param MeasuringRange            struct MeasuringRange. Set the width of the sensor measuring output range {minOutputValue, maxOutputValue, measuringOffset}
         * @param _signalType               enum signalType. Set the signal type: currentSignal or voltageSignal
         * @param SignalRange               struct SignalRange. Set width of the sensor signal range {minSignalValue, maxSignalValue, signalOffset}
         * @param measuringMinIsSignalMin   bool. Set true if min measuring value is equivalent to min signal value. Set false otherwise
         */
        SensorClass(const sensorType& _sensorType, const MeasuringRange& mRange, const signalType& _signalType, 
                    const SignalRange& sRange, bool _measuringMinIsSignalMin);

        /**
         * @brief Construct a new Sensor Class object without sensor type
         * 
         * @param MeasuringRange            struct MeasuringRange. Set the width of the sensor output range {minOutputValue, maxOutputValue, measuringOffset}
         * @param _signalType               enum signalType. Set the signal type: currentSignal or voltageSignal
         * @param SignalRange               struct SignalRange. Set width of the sensor signal range {minSignalValue, maxSignalValue, signalOffset}
         * @param measuringMinIsSignalMin   bool. Set true if min measuring value is equivalent to min signal value. Set false otherwise
         */
        SensorClass(const MeasuringRange& mRange, const signalType& _signalType, 
                    const SignalRange& sRange, bool _measuringMinIsSignalMin);

        /**
         * @brief Destroy the Sensor Class object
         * 
         */
        ~SensorClass();

        /**
         * @brief Get the Min Output Value object
         * 
         * @return float min measuring value
         */
        float getMinOutputValue() const;

        /**
         * @brief Get the Max Output Value object
         * 
         * @return float max measuring value
         */
        float getMaxOutputValue() const;

        /**
         * @brief Get the Output Value Offset object
         * 
         * @return float offset of the measuring value
         */
        float getMeasurandOffset() const;

        /**
         * @brief Get the Min Signal Value object
         * 
         * @return float min signal value in mA or V
         */
        float getMinSignalValue() const;

        /**
         * @brief Get the Max Signal Value object
         * 
         * @return float max signal value in mA or V
         */
        float getMaxSignalValue() const;

        /**
         * @brief Get the Signal Value Offset object
         * 
         * @return float offset of the signal value
         */
        float getSignalValueOffset() const;

        /**
         * @brief Get the Range of the sensor object
         * 
         * @return range of the sensor
         */
        float getRange() const;

        /**
         * @brief Get function for the resolution
         * 
         * @return resolution ( (measured Unit)/ (signal unit) ) if the output signal is a current type, the unit is mA, if it is a voltage type, the unit is V
         */
        float getResolution() const;

        /**
         * @brief Get the zero signal Value
         * 
         * @return zero signal value (mA or V) when the measured value is 0 (measured unit)
         */
        float getZeroValue() const;

        /**
         * @brief classwide return of all variables, this pointer probably not necessary
         * 
         * @return * void 
         */
        void printValues();

    protected:
        float res = {0.0};                  //Resolution of the sensor
        float deltaValue = {0.0};           //Range of the sensor
        float zeroValue = {0.0};            //Zero value of the sensor

        MeasuringRange myMeasuringRange;    //Measuring range of the sensor
        SignalRange mySignalRange;          //Signal range of the sensor   

};

#endif