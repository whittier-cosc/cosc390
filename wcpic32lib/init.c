/*
 *  @file   init.c
 */

#include "private/clocks.h"

unsigned _sysclk, _pbclk;

/**
 *  Tells wcpic32lib our system clock and peripheral bus clock rates.
 *
 *  This function must be called before using any other wcpic32lib functions.
 *
 *  Example:
 *
 *      wclib_init(SYSCLK, PBCLK);
 */
void wclib_init(unsigned sysclk, unsigned pbclk) {
    _sysclk = sysclk;
    _pbclk = pbclk;
}
