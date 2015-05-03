// Copyright (C) 2015 Francis Bergin

#ifndef ROTARYENCODER_H_
#define ROTARYENCODER_H_

extern BYTE rot_enc_change;
extern float desired_temp;

// function prototypes
extern void check_rotary (void);

extern WORD desired_read_temp1 ( void );
extern WORD desired_read_temp2 ( void );
extern WORD desired_read_temp3 ( void );

#endif // ROTARYENCODER_H_
