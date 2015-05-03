// Copyright (C) 2015 Francis Bergin

#include "includes.h"

DWORD adc0_average;
DWORD adc0_sum;
WORD adc0_check;
WORD x;

WORD temp_value1;
WORD temp_value2;
WORD temp_value3;

BYTE adc0_valid = 0;

float adc0_temp;

float adc0_calc_slope = (200.0 / 921.6);
float adc0_calc_intercept = (550.0 / 9.0);

float adc0_heat_compensation = 15.0;
float adc0_heat_comp_custom = 5.0;

BYTE adc0_pwr_src = 1;

void adc0_init(void) {
    ADCSRA = 0x80;
    ADMUX = 0x40;
}

WORD adc0_read(void) { return adc0_average; }

void adc0_data(void) {
    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait until conversion complete
    while (!(ADCSRA & (1 << ADIF)))
        ;

    // CAUTION: READ ADCL BEFORE ADCH!!!
    adc0_check = ((ADCL) | ((ADCH) << 8));
    adc0_sum += ((ADCL) | ((ADCH) << 8));

    if (adc0_valid == 0) {
        if (x == 511) {
            adc0_valid = 1;

            adc0_average = adc0_sum >> 9;
            // equation according to AD22100 characteristics
            adc0_temp = (adc0_average * adc0_calc_slope) - adc0_calc_intercept;
            // to compensate for board heat
            adc0_temp -= adc0_heat_compensation;
            adc0_sum = 0;
            x = 0;
        } else
            x++;
    }

    if (adc0_valid == 1) {
        if (x == 4095) {
            adc0_average = adc0_sum >> 12;
            // equation according to AD22100 characteristics
            adc0_temp = (adc0_average * 0.2172) - 61.07;
            // to compensate for board heat
            adc0_temp -= adc0_heat_compensation;
            adc0_sum = 0;
            x = 0;
        } else
            x++;
    }
}

WORD adc_read_temp1(void) {
    temp_value1 = adc0_temp / 10;
    return temp_value1;
}

WORD adc_read_temp2(void) {
    WORD temp0, temp1;

    temp0 = adc0_temp / 10;
    temp1 = temp0 * 10;
    temp_value2 = adc0_temp - temp1;

    return temp_value2;
}

WORD adc_read_temp3(void) {
    WORD temp0, temp1;

    temp0 = adc0_temp;
    temp1 = temp0 * 10;
    temp_value3 = (adc0_temp * 10) - temp1;

    return temp_value3;
}

WORD adc_read_compensation1(void) {
    temp_value1 = adc0_heat_compensation / 10;
    return temp_value1;
}

WORD adc_read_compensation2(void) {
    WORD temp0, temp1;

    temp0 = adc0_heat_compensation / 10;
    temp1 = temp0 * 10;
    temp_value2 = adc0_heat_compensation - temp1;

    return temp_value2;
}

WORD adc_read_compensation3(void) {
    WORD temp0, temp1;

    temp0 = adc0_heat_compensation;
    temp1 = temp0 * 10;
    temp_value3 = (adc0_heat_compensation * 10) - temp1;

    return temp_value3;
}
