#ifndef CALLBACK_H
#define CALLBACK_H

//include
#include "include.h"
//function
void callback_profile_state(gaprole_States_t newState);
void callback_charactersitic_value(uint8 paramID);
void callback_rssi(uint16 connHandle,int8 rssi);
uint8 callback_event(gapRoleEvent_t *pEvent);
void callback_passcode(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs);
void callback_pairstate(uint16 connHandle,uint8 state,uint8 status);
void callback_npi(uint8 port,uint8 events);

#endif