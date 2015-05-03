// Copyright (C) 2015 Francis Bergin

#ifndef RELAYS_H_
#define RELAYS_H_

extern WORD relays_cnt0;

extern BYTE hvac_flag_sys;
extern BYTE hvac_flag_auto;
extern BYTE hvac_flag_heat_src;

extern BYTE hvac_heat_flag;
extern BYTE hvac_ac_flag;
extern BYTE hvac_vent_flag;

extern BYTE hvac_heat;
extern BYTE hvac_ac;
extern BYTE hvac_vent;

// function prototypes
extern void control_relays(void);

#endif // RELAYS_H_
