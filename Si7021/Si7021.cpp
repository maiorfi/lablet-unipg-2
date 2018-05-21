#include "Si7021.h"

#define Si7021_Measure_RH (0xE5) // command for RH=Relative Humidity
#define Si7021_ReadPrevTemp (0xE0) // command for Reading Temp from Previous RH Measurement
#define Si7021_Measure_AMB_TEMP (0xE3) // command for Ambient Temperature measurement

//*****************************************************************
// HEAVILY MODIFIED MLX90614 library from
// http://mbed.org/users/aquahika/libraries/MLX90614/lsixz6
//
//*****************************************************************

Si7021::Si7021(I2C* i2c,int addr){

    this->i2caddress = addr; // this->i2caddress is a member variable,
                             // the RHS is the formal parameter
    this->i2c = i2c; // this->i2c is a member variable,
                     // the RHS is the formal parameter
    
}

bool Si7021::getRH(float* RH_val){

    /* Definition of member function through "Scope Operator" "::"
     * ("operatore di risoluzione dell'ambito",
     * che serve a dire a quale classe appartiene il metodo che sto definendo
     *     ====>  [std::] is the standard namespace
     */  

    char data_write[2];
    char data_read[3];  // was:  char data_read[2], but the PEC should also be read!!!
    int status;
    unsigned char crc;

    // read Relative Humidity
    data_write[0] = Si7021_Measure_RH;
    status = i2c->write(i2caddress, data_write, 1, true);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }

    // status = i2c->read(Si7021_ADDR, data_read, 2); // but the PEC should also be read!
    // we could also write "Si7021_ADDR | 0x01" but this is done by the read function
    status = i2c->read(i2caddress, data_read, 3, false);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }

    // printf("%d\n", (int)data_read[0]); // data byte high is received first
    // printf("%d - ", (int)data_read[1]); // data byte low is received after data byte high

    int t=0;
    t=(int)data_read[0];
    t=t<<8;
    t = t|((int)data_read[1]);
    *RH_val = 125*(float)t/65536-6.0;
    printf("in-the-library RH=%6.2f %%\n", *RH_val);
    /* Simulation of CRC,
     * calculated using only the data bytes (not explained in the data sheet) */
    crc = 0x00; // crc initialization
    crc8(&crc, data_read[0]);
    crc8(&crc, data_read[1]);

    printf("CRC from Si7021 = 0x%2x and calculated CRC = 0x%2x\r\n",(int)data_read[2],(int)crc);

    return true; //load data successfully, return true 
}


bool Si7021::getPrevTemp(float* temp_val){

    /* Definition of member function through "Scope Operator" "::"
     * ("operatore di risoluzione dell'ambito",
     * che serve a dire a quale classe appartiene il metodo che sto definendo
     *     ====>  [std::] is the standard namespace
     */  

    char data_write[2];
    char data_read[3];  // was:  char data_read[2], but the PEC should also be read!!!
    int status;
    // unsigned char crc;

    // Read ambient temperature (in degrees Celsius)
    // previously read during RH measurement
    data_write[0] = Si7021_ReadPrevTemp;
    status = i2c->write(i2caddress, data_write, 1, true);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }
    // From data sheet:
    // The checksum output is not available with the 0xE0 command.
    // Then read only 2 bytes
    status = i2c->read(i2caddress, data_read, 2, false);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }
        
    // printf("%d - ", (int)data_read[0]); // data byte high is received first
    // printf("%d\n", (int)data_read[1]); // data byte low is received after data byte high

    int t=(int)data_read[0];
    t=t<<8;
    t = t|((int)data_read[1]);
    *temp_val = 175.72*(float)t/65536-46.85;
    printf("in-the-library Temperature:%6.2f degC\n", *temp_val);
    
    printf("The checksum output is not available with the 0xE0 command\r\n");
    
    return true; //load data successfully, return true 
}

bool Si7021::getTemp(float* temp_val){

    /* Definition of member function through "Scope Operator" "::"
     * ("operatore di risoluzione dell'ambito",
     * che serve a dire a quale classe appartiene il metodo che sto definendo
     *     ====>  [std::] is the standard namespace
     */  

    char data_write[2];
    char data_read[3];  // was:  char data_read[2], but the PEC should also be read!!!
    int status;
    unsigned char crc;

    // Read ambient temperature (in degrees Celsius)
    data_write[0] = Si7021_Measure_AMB_TEMP;
    status = i2c->write(i2caddress, data_write, 1, true);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }

    status = i2c->read(i2caddress, data_read, 3, false);
    if(status != 0){
        printf("Write failed\n");
        return false;
    }

    // printf("%d - ", (int)data_read[0]); // data byte high is received first
    // printf("%d\n", (int)data_read[1]); // data byte low is received after data byte high

    int t=(int)data_read[0];
    t=t<<8;
    t = t|((int)data_read[1]);
    *temp_val = 175.72*(float)t/65536-46.85;
    printf("in-the-library Temperature:%6.2f degC\n", *temp_val);

    /* Simulation of CRC,
     * calculated using only the data bytes (not explained in the data sheet) */
    crc = 0x00; // crc initialization
    crc8(&crc, data_read[0]);
    crc8(&crc, data_read[1]);

    printf("CRC from Si7021 = 0x%2x and calculated CRC = 0x%2x\r\n",(int)data_read[2],(int)crc);
    
    return true; //load data successfully, return true 
}
