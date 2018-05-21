#include "mbed.h"

/* header files (.h) contain the declarations (or prototypes)
 * of classes (i.e. their interface)
 */

// Si7021 RH and Temperature sensor Library

//*****************************************************************
// HEAVILY MODIFIED MLX90614 library from
// http://mbed.org/users/aquahika/libraries/MLX90614/lsixz6
//
//*****************************************************************

/**An Interface for Si7021
* 
* @code
* //Print temperature data
* #include "mbed.h"
* #include "Si7021.h"
*
* I2C i2c(p28,p27);   //sda,scl
* Si7021 RH&T(&i2c);
* float RH, temp;
*
* void main(void){
*   if(RH&T.getRH(&RH)){
*       printf("RH : %f %%\r\n",RH);
*   }
*   if(RH&T.getPrevTemp(&temp)){
*       printf("Temperature : %f degC\r\n",temp);
*   }
*   wait(2.0);
*
* }
* @endcode
*/

// prototype of crc8 function
void crc8(unsigned char *crc, unsigned char m);

class Si7021{    // prototype of "Si7021" class

    public:
        /** Create Si7021 interface, initialize with selected I2C port and address.
        *
        * @param i2c I2C device pointer
        * @param addr Device address(default=0x80)  
        */
        
        // prototype of Si7021 constructor
        Si7021(I2C* i2c,int addr);
        
        /* Prototypes of member functions
         * Get RH and Temperature from Si7021. 
         * @param RH_val, prevTemp_val and temp_val return variable pointer
         * @return 0 on success (ack), or non-0 on failure (nack)
         */
        bool getRH(float* RH_val);
        bool getPrevTemp(float* temp_val);
        bool getTemp(float* temp_val1);
        
    private: // start of private section
       // private MEMBER variables declaration 
       I2C* i2c;  // Construction of an instance
                  // of type i2c pointer of class I2C
       int i2caddress; // declaration of an integer

    //NB: please note the mandatory ';' at the end of the prototype!!
};