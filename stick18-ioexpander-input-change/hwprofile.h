/**
 *  @file   hwprofile.h
 *
 *  @brief  Defines `SYSCLK` and `PBCLK` constants representing the system
 *          clock speed and the peripheral bus clock speed of the PIC32.
 *
 *          Many of the drivers in our library include this file and set
 *          communication speeds based on SYSCLK and/or PBCLK, so be sure
 *          to update this file appropriately if you change the system clock
 *          or peripheral bus clock speed (by changing configuration settings
 *          in config.h, for example).
 *
 *  @author Jeff Lutgen
 */

#ifndef HWPROFILE_H
#define HWPROFILE_H

#define SYSCLK 40000000 ///< 40 MHz system clock
#define PBCLK  SYSCLK   ///< 40 MHz peripheral bus clock

#endif // HWPROFILE_H
