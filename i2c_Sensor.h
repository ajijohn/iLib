#ifndef i2c_interface_h
#define i2c_interface_h

/**<  Abstract Class as Interface for sensor-libs */
/**<  with basic functions that have to be implemented */

class i2cSensor
{

public:
    /**< TODO: constructur is not usefull? */

    /**< TODO: Try const functions */

    /**< TODO: new functions: wait for value, check for new value */

    /**< declaring prototypes */
    virtual void    setEnabled(uint8_t enable=1) = 0;     // Enable Sensor or set it to Standby (1 or 0)

    virtual void    reset(void) = 0;                    // trigger a software-reboot of the sensor

    virtual uint8_t initialize(void) = 0;               // set up the sensor for basic operation, IF found at address
    //uint8_t         init(void) {initialize();};         // short form of the function above

    /**< check for new data, return 1 when Measurement is ready */
    virtual uint8_t checkMeasurement(void) = 0;

    /**<  wait (and check) for new data in a busy-loop, TODO: give functionpointer to sleepfunction */
    virtual uint8_t awaitMeasurement(void) = 0;

    /**< new getValue */
    //virtual void    getMeasurement(int16_t xyz_raw[])

    /**< TODO: solveable with anonymous (void) pointers or similar reference */
    //virtual void    getValue(uint8_t buffer[]) = 0;     // get the main values the sensor was made for (barometer gives pressure)

protected:
    // instance gets control over
    //uint8_t _address;
private:
    // instance has no control over these
};

class manualSensor
{
public:
/**< only used when in manual/standby mode */
    virtual void    triggerMeasurement(void) = 0;
};

#endif
