#ifndef ROTARYENCODER_H_
#define ROTARYENCODER_H_

extern void check_rotary (void);
extern BYTE rot_enc_change;

extern WORD desired_read_temp1 ( void );
extern WORD desired_read_temp2 ( void );
extern WORD desired_read_temp3 ( void );

extern float desired_temp;

#endif // ROTARYENCODER_H_
