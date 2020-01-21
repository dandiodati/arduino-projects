//******************************************************************************************
//  File: PS_Voltage.h
//  Authors: Dan G Ogorchock & Daniel J Ogorchock (Father and Son)
//
//  Summary:  PS_Voltage is a class which implements the SmartThings "Voltage Measurement" device capability.
//			  It inherits from the st::PollingSensor class.  The current version uses an analog input to measure the 
//			  voltage on an anlog input pin and then scale it to engineering units.
//
//			  The last four arguments of the constructor are used as arguments to an Arduino map() function which 
//			  is used to scale the analog input readings (e.g. 0 to 1024) to Engineering Units before sending to SmartThings. 
//
//			  Create an instance of this class in your sketch's global variable section
//			  For Example:  st::PS_Voltage sensor1(F("voltage1"), 120, 0, PIN_VOLTAGE, 0, 1023, 0.0, 5.0);
//
//			  st::PS_Voltage() constructor requires the following arguments
//				- String &name - REQUIRED - the name of the object - must match the Groovy ST_Anything DeviceType tile name
//				- long interval - REQUIRED - the polling interval in seconds
//				- long offset - REQUIRED - the polling interval offset in seconds - used to prevent all polling sensors from executing at the same time
//				- byte pin - REQUIRED - the Arduino Pin to be used as an analog input
//				- double s_l - OPTIONAL - first argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - minimum raw AI value
//				- double s_h - OPTIONAL - second argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - maximum raw AI value
//				- double m_l - OPTIONAL - third argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - Engineering Unit Min (or Max if inverting)
//				- double m_h - OPTIONAL - fourth argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - Engineering Unit Max (or Min if inverting)
//				- byte numSamples - OPTIONAL - defaults to 1, number of analog readings to average per scheduled reading of the analog input
//				- byte filterConstant - OPTIONAL - Value from 5% to 100% to determine how much filtering/averaging is performed 100 = none (default), 5 = maximum
//
//            Filtering/Averaging
//
//				Filtering the value sent to ST is performed per the following equation
//
//				filteredValue = (filterConstant/100 * currentValue) + ((1 - filterConstant/100) * filteredValue) 
//
//----------------------------------------------------------------------------------------------------------------------------------------------
//			  st::PS_Voltage() has a second constructor which includes a 3rd order polynomial compensation algorithm.
//
//			  Create an instance of this class in your sketch's global variable section
//			  For Example:  static st::PS_Voltage sensor5(F("voltage1"), 5, 1, PIN_VOLTAGE_1, -40, 140, 0, 4095, 20, 75, -0.000000025934, 0.0001049656215,  0.9032840665333,  204.642825355678);
//
//              The following arguments all all REQUIRED in order to use the Compensation Algorithm.
//				- String &name - REQUIRED - the name of the object - must match the Groovy ST_Anything DeviceType tile name
//				- long interval - REQUIRED - the polling interval in seconds
//				- long offset - REQUIRED - the polling interval offset in seconds - used to prevent all polling sensors from executing at the same time
//				- byte pin - REQUIRED - the Arduino Pin to be used as an analog input
//				- double s_l - OPTIONAL - first argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - minimum raw AI value
//				- double s_h - OPTIONAL - second argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - maximum raw AI value
//				- double m_l - OPTIONAL - third argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - Engineering Unit Min (or Max if inverting)
//				- double m_h - OPTIONAL - fourth argument of Arduino map(s_l,s_h,m_l,m_h) function to scale the output - Engineering Unit Max (or Min if inverting)
//				- byte numSamples - REQUIRED - number of analog readings to average per scheduled reading of the analog input
//				- byte filterConstant - REQUIRED - Value from 5% to 100% to determine how much filtering/averaging is performed 100 = none, 5 = maximum
//				- double Coeff1 - REQUIRED - 3rd order polynomial coefficient #1
//				- double Coeff2 - REQUIRED - 3rd order polynomial coefficient #2
//				- double Coeff3 - REQUIRED - 3rd order polynomial coefficient #3
//				- double Coeff4 - REQUIRED - 3rd order polynomial coefficient #4
//
//			  3rd order Plynomial Compensation Algorithm (useful for correcting non-linear analog to digital converters)
//
// 				CompensatedValue = Coeff1 * rawAnalogInput^3 + Coeff2 * rawAnalogInput^2 + Coeff3 * rawAnalogInput + Coeff4
//
//			  This class supports receiving configuration data from the SmartThings cloud via the ST App.  A user preference
//			  can be configured in your phone's ST App, and then the "Configure" tile will send the data for all sensors to 
//			  the ST Shield.  For PollingSensors, this data is handled in the beSMart() function.
//
//			  TODO:  Determine a method to persist the ST Cloud's Polling Interval data
//
//  Change History:
//
//    Date        Who            What
//    ----        ---            ----
//    2015-04-19  Dan & Daniel   Original Creation
//    2017-08-18  Dan Ogorchock  Modified to return floating point values instead of integer
//    2017-08-31  Dan Ogorchock  Added oversampling optional argument to help reduce noisy signals
//    2017-08-31  Dan Ogorchock  Added filtering optional argument to help reduce noisy signals
//    2017-09-01  Dan Ogorchock  Added 3rd order polynomial nonlinear correction compensation
//    2018-06-24  Dan Ogorchock  Improved documentation / comments (above)
//
//
//******************************************************************************************
#ifndef ST_PS_VOLTAGE_H
#define ST_PS_VOLTAGE_H

#include "PollingSensor.h"

namespace st
{
	class PS_Voltage: public PollingSensor
	{
		private:
			byte m_nAnalogInputPin;
			float m_fSensorValue;
			double SENSOR_LOW, SENSOR_HIGH, MAPPED_LOW, MAPPED_HIGH;
			int m_nNumSamples;
			float m_fFilterConstant;        //Filter constant % as floating point from 0.00 to 1.00
			double m_dCoeff1, m_dCoeff2, m_dCoeff3, m_dCoeff4;  //3rd order polynomial nonlinear correction compensation coefficients
			bool m_bUseCompensation;

		public:
			//constructor - called in your sketch's global variable declaration section
			PS_Voltage(const __FlashStringHelper *name, unsigned int interval, int offset, byte analogInputPin, double s_l=0, double s_h=1023, double m_l=0, double m_h=5000, int NumSamples=1, byte filterConstant = 100);
			
			//constructor with 3rd order polynomial nonlinear correction compensation coefficients - called in your sketch's global variable declaration section
			PS_Voltage(const __FlashStringHelper *name, unsigned int interval, int offset, byte analogInputPin, double s_l, double s_h, double m_l, double m_h, int NumSamples, byte filterConstant, double Coeff1, double Coeff2, double Coeff3, double Coeff4);

			//destructor
			virtual ~PS_Voltage();
			
			//SmartThings Shield data handler (receives configuration data from ST - polling interval, and adjusts on the fly)
			virtual void beSmart(const String &str);

			//function to get data from sensor and queue results for transfer to ST Cloud 
			virtual void getData();
			
			//gets
			inline byte getPin() const {return m_nAnalogInputPin;}
			inline float getSensorValue() const {return m_fSensorValue;}
				
			//sets
			void setPin(byte pin);
	};
}
#endif
