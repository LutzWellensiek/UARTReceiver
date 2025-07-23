#include "SensorClasses.h"


//SensorClass::SensorClass(sensorType _sensorType, const float measuringRange[2], signalType _signalType, const float outputSignalRange[2], bool measuringMinIsSignalMin)
//const float _minOutputValue, const float _maxOutputValue
SensorClass::SensorClass(const sensorType& _sensorType, const MeasuringRange& mRange, const signalType& _signalType, 
                         const SignalRange& sRange, bool _measuringMinIsSignalMin) :  myMeasuringRange(mRange), mySignalRange(sRange)
{
    //Initialize class attributes
    mySensorType = _sensorType;
    // minOutputValue = measuringRange[0];
    // maxOutputValue = measuringRange[1];
    //myMeasuringRange.minOutputValue = _minOutputValue;
    //myMeasuringRange.maxOutputValue = _maxOutputValue;
    
    mySignalType = _signalType;
    // minSignalValue = outputSignalRange[0];
    // maxSignalValue = outputSignalRange[1];
    //mySignalRange.minSignalValue = _minSignalValue;
    //mySignalRange.maxSignalValue = _maxSignalValue;

    measuringMinIsSignalMin = _measuringMinIsSignalMin;
    //myInputPort = inputPort; 
    
    //Call all calc functions
    calcRange();
    calcResolution();
    calcZeroValue();
}

SensorClass::SensorClass(const MeasuringRange& mRange, const signalType& _signalType,
                         const SignalRange& sRange, bool _measuringMinIsSignalMin) : myMeasuringRange(mRange), mySignalRange(sRange) {
    //Initialize class attributes
    //minOutputValue = measuringRange[0];
    //maxOutputValue = measuringRange[1];
    //myMeasuringRange.minOutputValue = _minOutputValue;
    //myMeasuringRange.maxOutputValue = _maxOutputValue;

    mySignalType = _signalType;
    // minSignalValue = outputSignalRange[0];
    // maxSignalValue = outputSignalRange[1];
    // minSignalValue = _minSignalValue;
    // maxSignalValue = _maxSignalValue;
    //mySignalRange.minSignalValue = _minSignalValue;
    //mySignalRange.maxSignalValue = _maxSignalValue;

    measuringMinIsSignalMin = _measuringMinIsSignalMin;
    //myInputPort = inputPort;

    //Call all calc functions
    calcRange();
    calcResolution();
    calcZeroValue();
}

SensorClass::~SensorClass() {
}

float SensorClass::getMinOutputValue() const{
    return myMeasuringRange.minOutputValue;
}

float SensorClass::getMaxOutputValue() const{
    return myMeasuringRange.maxOutputValue;
}

float SensorClass::getMeasurandOffset() const{
    return myMeasuringRange.measuringOffset;
}

float SensorClass::getMinSignalValue() const{
    return mySignalRange.minSignalValue;
}

float SensorClass::getMaxSignalValue() const{
    return mySignalRange.maxSignalValue;
}

float SensorClass::getSignalValueOffset() const{
    return mySignalRange.signalOffset;
}

void SensorClass::calcRange(){
    deltaValue = abs(myMeasuringRange.maxOutputValue - myMeasuringRange.minOutputValue);
}

float SensorClass::getRange() const{
    return deltaValue;
}

void SensorClass::calcResolution(){
    res = deltaValue / (mySignalRange.maxSignalValue - mySignalRange.minSignalValue);
}

float SensorClass::getResolution() const{
    return res;
}

void SensorClass::calcZeroValue(){
    //Condition to map the correct OutputValue to the signalValue and to calculate the correct zeroValue
    if (measuringMinIsSignalMin)
    {
        zeroValue = abs(myMeasuringRange.minOutputValue) / res + mySignalRange.minSignalValue;
    } else {
        zeroValue = abs(myMeasuringRange.maxOutputValue) / res + mySignalRange.minSignalValue;
    }        
}

float SensorClass::getZeroValue() const{
    return zeroValue;
}

void SensorClass::printValues()
{
    SerialMon.print("minOutputValue: ");
    SerialMon.println(this->myMeasuringRange.minOutputValue);
    SerialMon.print("maxOutputValue: ");
    SerialMon.println(this->myMeasuringRange.maxOutputValue);
    SerialMon.print("deltaValue: ");
    SerialMon.println(this->deltaValue);
    SerialMon.print("res: ");
    SerialMon.println(this->res);
    SerialMon.print("zero: ");
    SerialMon.println(this->zeroValue);
    SerialMon.println("");
}