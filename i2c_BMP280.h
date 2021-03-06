#ifndef i2c_bmp280_h
#define i2c_bmp280_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the BMP280-Sensor

 CONSUMPTION: standby 0.5 �A, measure 4.2@1Hz, 260-1120�A

 ONE-TIME-MEASURE: disable sensor, [start measurement, wait, read ] ...
 AUTO-Measure: enable sensor, start measurement, [read, read ] ...


########################################################################  */

class BMP280 : public i2cSensor, public manualSensor
{

    /** ######### Register-Map ################################################################# */

#define BMP_ADDRESS 	            0x76

// CALIBRATION DATA, 25 Register. 0x88 - 0xA1
#define REG_DIG_T1                  0x88    // watch out - switched MSB/LSB
#define REG_DIG_T2                  0x8A
#define REG_DIG_T3                  0x8C
#define REG_DIG_P1                  0x8E
#define REG_DIG_P2                  0x90
#define REG_DIG_P3                  0x92
#define REG_DIG_P4                  0x94
#define REG_DIG_P5                  0x96
#define REG_DIG_P6                  0x98
#define REG_DIG_P7                  0x9A
#define REG_DIG_P8                  0x9C
#define REG_DIG_P9                  0x9E

#define REG_ID						0xD0
#define		VAL_ID					0x58

#define REG_RESET					0xE0
#define 	VAL_RESET				0xB6 	// write it to trigger POR

#define REG_STATUS					0xF3
#define 	MSK_STATUS_MEASURING	(1<<3)	// 1 when conversion is running
#define 	MSK_STATUS_IMUPDATE		(1<<0)	// 1 when NVM data is copied to image registers

#define REG_CTRL_MEAS				0xF4
#define 	MSK_CTRL_OSRS_T			B11100000
#define 		VAL_CTRL_OSRS_T00	B00000000 // skip measurement
#define 		VAL_CTRL_OSRS_T01	B00100000 // 1x (no oversampling)
#define 		VAL_CTRL_OSRS_T02	B01000000 // 2x --> 17bit, 2m�C
#define 		VAL_CTRL_OSRS_T04	B01100000 // 4x  	(brings no improvement)
#define 		VAL_CTRL_OSRS_T08	B10000000 // 8x		(brings no improvement)
#define 		VAL_CTRL_OSRS_T16	B10100000 // 16x	(brings no improvement)
#define 	MSK_CTRL_OSRS_P			B00011100
#define 		VAL_CTRL_OSRS_P00	B00000000 // skip measurement
#define 		VAL_CTRL_OSRS_P01	B00000100 // 1x (no oversampling)
#define 		VAL_CTRL_OSRS_P02	B00001000 // 2x
#define 		VAL_CTRL_OSRS_P04	B00001100 // 4x
#define 		VAL_CTRL_OSRS_P08	B00010000 // 8x
#define 		VAL_CTRL_OSRS_P16	B00010100 // 16x --> 20 bit, 0.16 Pa
#define 	MSK_CTRL_MODE			B00000011
#define 		VAL_MODE_SLEEP		B00000000	// low power
#define 		VAL_MODE_FORCED		B00000001	// manual
#define 		VAL_MODE_NORMAL		B00000011 	// automatic

#define REG_CONFIG					0xF5
#define 	MSK_CONFIG_T_SB			B11100000
#define 		VAL_SB_0000			B00000000
#define 		VAL_SB_0062			B00100000
#define 		VAL_SB_0125			B01000000
#define 		VAL_SB_0250			B01100000
#define 		VAL_SB_0500			B10000000
#define 		VAL_SB_1000			B10100000
#define 		VAL_SB_2000			B11000000
#define 		VAL_SB_4000			B11100000
#define 	MSK_CONFIG_FILTER		B00011100
#define 		VAL_FILTER_00		B00000000	// full BW
#define 		VAL_FILTER_02 		B00000100	// 0.223 * ODR
#define 		VAL_FILTER_04 		B00001000	// 0.092 * ODR
#define 		VAL_FILTER_08 		B00001100	// 0.042 * ODR
#define 		VAL_FILTER_16 		B00010000	// 0.021 * ODR
#define 	MSK_CONFIG_SPI3W_EN		B00000001	// 1 = activate SPI-Mode

#define REG_PRESS_MSB				0xF7
#define REG_PRESS_LSB				0xF8
#define REG_PRESS_XLSB				0xF9 	// bit 4-7 usable

#define REG_TEMP_MSB				0xFA
#define REG_TEMP_LSB				0xFB
#define REG_TEMP_XLSB				0xFC	// bit 4-7 usable

private:
    uint16_t    dig_T1, dig_P1;
    int16_t     dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    int32_t     tFine;

    /** ######### function definition ################################################################# */

public:

    BMP280(void)
    {
        //_address = BMP_ADDRESS;
    };

    /**< Enable / Disable the Sensor */
    void    setEnabled(uint8_t enable = 1)
    {
        if (enable) enable=VAL_MODE_NORMAL;
        i2c.setRegister(BMP_ADDRESS,REG_CTRL_MEAS, MSK_CTRL_MODE, enable);
    };

    /**< read Enable / Disable - Status */
    uint8_t getEnabled()
    {
        return (3 & i2c.readByte(BMP_ADDRESS,REG_CTRL_MEAS));
    };

    /**< do a software reset */
    void    reset()
    {
        i2c.writeByte(BMP_ADDRESS,REG_RESET, VAL_RESET);
    };

    /**<  */
    void setPressureOversampleRatio(uint8_t sampleRatio = 16)
    {
        if 		(sampleRatio > 15) 	sampleRatio = VAL_CTRL_OSRS_P16;
        else if (sampleRatio > 7)	sampleRatio = VAL_CTRL_OSRS_P08;
        else if (sampleRatio > 3)	sampleRatio = VAL_CTRL_OSRS_P04;
        else if (sampleRatio > 1)	sampleRatio = VAL_CTRL_OSRS_P02;
        else if (sampleRatio > 0)	sampleRatio = VAL_CTRL_OSRS_P01;
        else  						sampleRatio = VAL_CTRL_OSRS_P00; // disable!!!
        i2c.setRegister(BMP_ADDRESS,REG_CTRL_MEAS, MSK_CTRL_OSRS_P, sampleRatio);
    };

    void setTemperatureOversampleRatio(uint8_t sampleRatio = 2)
    {
        if 		(sampleRatio > 15) 	sampleRatio = VAL_CTRL_OSRS_T16;
        else if (sampleRatio > 7)	sampleRatio = VAL_CTRL_OSRS_T08;
        else if (sampleRatio > 3)	sampleRatio = VAL_CTRL_OSRS_T04; // more isn't better
        else if (sampleRatio > 1)	sampleRatio = VAL_CTRL_OSRS_T02; // 2 should be maximum
        else if (sampleRatio > 0)	sampleRatio = VAL_CTRL_OSRS_T01;
        else  						sampleRatio = VAL_CTRL_OSRS_T00; // disable!!!
        i2c.setRegister(BMP_ADDRESS,REG_CTRL_MEAS, MSK_CTRL_OSRS_T, sampleRatio);
    };

    void setFilterRatio(uint8_t filterRatio = 0)
    {
        if 		(filterRatio > 15) 	filterRatio = VAL_FILTER_16;
        else if (filterRatio > 7)	filterRatio = VAL_FILTER_08;
        else if (filterRatio > 3)	filterRatio = VAL_FILTER_04;
        else if (filterRatio > 1)	filterRatio = VAL_FILTER_02;
        else  						filterRatio = VAL_FILTER_00; // disable!!!
        i2c.setRegister(BMP_ADDRESS,REG_CONFIG, MSK_CONFIG_FILTER, filterRatio);
    };

    void setStandby(uint16_t ms = 0)
    {
        uint8_t valueSB;
        if 		(ms > 3000) valueSB = VAL_SB_4000;
        else if (ms > 1500)	valueSB = VAL_SB_2000;
        else if (ms >  750)	valueSB = VAL_SB_1000;
        else if (ms >  350)	valueSB = VAL_SB_0500;
        else if (ms >  180)	valueSB = VAL_SB_0250;
        else if (ms >   90)	valueSB = VAL_SB_0125;
        else if (ms >   31)	valueSB = VAL_SB_0062;
        else  				valueSB = VAL_SB_0000; // disable!!!
        i2c.setRegister(BMP_ADDRESS,REG_CONFIG, MSK_CONFIG_T_SB, valueSB);
    }

    /**< initialize */
    uint8_t initialize()
    {
        if (i2c.probe(BMP_ADDRESS)==0) return 0;

        reset();
        delay(4);

        setPressureOversampleRatio(16);
        setTemperatureOversampleRatio(2);
        setFilterRatio();
        setStandby();
        i2c.setRegister(BMP_ADDRESS,REG_CONFIG, MSK_CONFIG_SPI3W_EN, 0);

        readTrimming();
/*
        Serial.println("");
        Serial.println(dig_T1);
        Serial.println(dig_T2);
        Serial.println(dig_T3);
*/
        setEnabled(1);
        return 1;
    };

    void readTrimming()
    {
        uint8_t value[2];
        i2c.read(BMP_ADDRESS, REG_DIG_T1, value, 2);
        dig_T1 = uint16_t((uint16_t(value[1]<<8)) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_T2, value, 2);
        dig_T2 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_T3, value, 2);
        dig_T3 = int16_t((value[1]<<8) | value[0]);

        i2c.read(BMP_ADDRESS, REG_DIG_P1, value, 2);
        dig_P1 = uint16_t((uint16_t(value[1]<<8)) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P2, value, 2);
        dig_P2 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P3, value, 2);
        dig_P3 = int16_t((value[1]<<8) | value[0]);

        i2c.read(BMP_ADDRESS, REG_DIG_P4, value, 2);
        dig_P4 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P5, value, 2);
        dig_P5 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P6, value, 2);
        dig_P6 = int16_t((value[1]<<8) | value[0]);

        i2c.read(BMP_ADDRESS, REG_DIG_P7, value, 2);
        dig_P7 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P8, value, 2);
        dig_P8 = int16_t((value[1]<<8) | value[0]);
        i2c.read(BMP_ADDRESS, REG_DIG_P9, value, 2);
        dig_P9 = int16_t((value[1]<<8) | value[0]);
    };


    /**< disables continious Mode! (enable(1)) */
    void triggerMeasurement(void)
    {
        i2c.setRegister(BMP_ADDRESS,REG_CTRL_MEAS, MSK_CTRL_MODE, VAL_MODE_FORCED);
    };

    uint8_t checkMeasurement(void)
    {
        return !(MSK_STATUS_MEASURING & i2c.readByte(BMP_ADDRESS, REG_STATUS));
    };

    /**<  if you started a measurement and want to actively wait for it to finish */
    uint8_t awaitMeasurement(void)
    {
        uint8_t _counter = 0;
        while(checkMeasurement()==0)
        {
            if(++_counter > 250) return 0; //Error out after max of 500ms for a read
            delay(2);
        }
        return 1; // Measurement finished
    };

    /**<  gives airpressure in Pascal */
    void getPressure(uint32_t& pascal)
    {
        uint8_t value[3];
        i2c.read(BMP_ADDRESS, REG_PRESS_MSB, value, 3);

        int32_t var1, var2, adc;
        adc     = (uint32_t( uint16_t(value[0] << 8) | value[1])<<4) | (value[2]>>4);
        var1 = (((int32_t)tFine)>>1) - (int32_t)64000;
        var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
        var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
        var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
        var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
        var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
        if (var1 == 0) return; // avoid exception caused by division by zero

        pascal = (((uint32_t)(((int32_t)1048576)-adc)-(var2>>12)))*3125;
        if (pascal < 0x80000000)    pascal = (pascal << 1) / ((uint32_t)var1);
        else                        pascal = (pascal / (uint32_t)var1) * 2;

        var1 = (((int32_t)dig_P9) * ((int32_t)(((pascal>>3) * (pascal>>3))>>13)))>>12;
        var2 = (((int32_t)(pascal>>2)) * ((int32_t)dig_P8))>>13;
        pascal = (uint32_t)((int32_t)pascal + ((var1 + var2 + dig_P7) >> 4));
    };

    void getPressure(float& pascal)
    {
        uint32_t iPascal;
        getPressure(iPascal);
        pascal = float(iPascal);
    }

    /**<  gives pressure-values */
    void getMeasurement(float& pascal)
    {
        getPressure(pascal);
    };

    /**<  gives the number of meters above sea level */
    void getAltitude(float& meter)
    {
        uint32_t iPascal;
        getPressure(iPascal);

        meter = 44330.0*(1-pow(float(iPascal)/101325.0,1.0/5.255));

    };

    /**<  gives temperature in degree celsius */
    void getTemperature(int32_t& millicelsius)
    {
        uint8_t value[3];
        i2c.read(BMP_ADDRESS, REG_TEMP_MSB, value, 3);

        int32_t var1, var2, adc;
        adc     = (uint32_t( uint16_t(value[0] << 8) | value[1])<<4) | (value[2]>>4);
        var1    = ((((adc>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
        var2    = (((((adc>>4) - ((int32_t)dig_T1)) * ((adc>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3))>>14;
        //var2 = ((adc>>4) - ((int32_t)dig_T1));
        //var2 = (((var2 * var2) >> 12) * ((int32_t)dig_T3))>>14;
        tFine   = var1 + var2;
        millicelsius = (tFine * 50 + 1280) >> 8;

    };

    void getTemperature(float& celsius)
    {
        int32_t iTemperature;
        getTemperature(iTemperature);
        celsius = float(iTemperature) / 1000;
    };

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//BMP280 bmp280 = BMP280();

#endif



