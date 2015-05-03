#ifndef __MSPI_H__
#define __MSPI_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com
 * Date : Tue Apr 21 2015
 **/

#if DEVICE_SPI

#include "mbed.h"
#include "rtos.h" // to use mutex
#include "SPI.h"

namespace mbed {

    class MSPI: public SPI
    {
        public:
            MSPI(PinName mosi, PinName miso, PinName sclk, Mutex & mtx)
                : SPI(mosi, miso, sclk),
                _mtx(mtx)
            {
            }

            virtual void acquireBus()
            {
                _mtx.lock();
                aquire(); // set freq, mode
            }

            virtual void releaseBus()
            {
                _mtx.unlock();
            }


        public:
            virtual ~MSPI() 
            {
            }
        private:
            Mutex & _mtx;
    };
} // namespace mbed

#endif  // DEVICE_SPI

#endif  // __MSPI_H__

