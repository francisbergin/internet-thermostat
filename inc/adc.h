

extern WORD adc0_read (void);
extern void adc0_init (void);
extern void adc0_data (void);
extern WORD adc_read_temp1(void);
extern WORD adc_read_temp2(void);
extern WORD adc_read_temp3(void);


extern WORD adc_read_compensation1(void);
extern WORD adc_read_compensation2(void);
extern WORD adc_read_compensation3(void);

extern float adc0_temp;
extern BYTE adc0_valid;


extern float adc0_calc_slope;
extern float adc0_calc_intercept;

extern float adc0_heat_compensation;
extern float adc0_heat_comp_custom;
extern BYTE adc0_pwr_src;