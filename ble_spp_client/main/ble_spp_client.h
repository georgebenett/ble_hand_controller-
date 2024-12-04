#ifndef SPP_CLIENT_DEMO_H
#define SPP_CLIENT_DEMO_H

void spp_client_demo_init(void);
float get_latest_voltage(void);
int32_t get_latest_rpm(void);
float get_latest_current_motor(void);
float get_latest_current_in(void);
float get_latest_amp_hours(void);
float get_latest_amp_hours_charged(void);

#endif // SPP_CLIENT_DEMO_H 