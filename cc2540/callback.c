#include "callback.h"

extern uint8 g_taskid;//Task ID for internal task/event processing
//extern bool g_timeout;//TRUE-timeout,FALSE-timein
extern bool g_init;//TRUE-central init finished,FALSE-central init unfinished
extern bool g_adv;//TRUE-adv enable,FALSE-adv disable
extern bool g_scan;//TRUE-scan start,FALSE-scan stop
extern uint8 g_scannum;//scan found number
extern gapDevRec_t g_scanlist[SCAN_MAXNUM];//scan list
extern multi_t g_central[NUM_CENTRAL];//central array
extern multi_t g_peripheral;//peripheral array
extern uint8 g_multi;//central array number

//Notification from the profile of a state change.
void callback_profile_state(gaprole_States_t newState)//peripheral
{
    uint8 own_address[B_ADDR_LEN];//6 bytes
    uint8 system_id[DEVINFO_SYSTEM_ID_LEN];//8 bytes
    
    switch(newState)
    {
    case GAPROLE_STARTED:
        GAPRole_GetParameter(GAPROLE_BD_ADDR,own_address);//Device's Address(read from controller)
        //use 6 bytes of device address for 8 bytes of system ID value
        system_id[0]=own_address[0];
        system_id[1]=own_address[1];
        system_id[2]=own_address[2];
        //set middle bytes to zero
        system_id[3]=0x00;
        system_id[4]=0x00;
        //shift three bytes up
        system_id[5]=own_address[3];
        system_id[6]=own_address[4];
        system_id[7]=own_address[5];
        DevInfo_SetParameter(DEVINFO_SYSTEM_ID,DEVINFO_SYSTEM_ID_LEN,system_id);//Set a Device system_id parameter
        //Display device address
        HalLcdWriteString("Peripheral Init",HAL_LCD_LINE_1);
        HalLcdWriteString(address2string(own_address),HAL_LCD_LINE_2);
        g_peripheral.status=STATE_IDLE;
        break;
    case GAPROLE_ADVERTISING:
        HalLcdWriteString("Peripheral Adv",HAL_LCD_LINE_1);
        break;
    case GAPROLE_CONNECTED:
        HalLcdWriteString("Peripheral Connect",HAL_LCD_LINE_1);
        g_peripheral.status=STATE_CONNECTED;
        GAPRole_GetParameter(GAPROLE_CONNHANDLE,&g_peripheral.handle_conn);
        break;
    case GAPROLE_WAITING:
        HalLcdWriteString("Peripheral Disconn",HAL_LCD_LINE_1);
        g_peripheral.status=STATE_IDLE;
        break;
    case GAPROLE_WAITING_AFTER_TIMEOUT:
        HalLcdWriteString("Peripheral Timeout",HAL_LCD_LINE_1);
        g_peripheral.status=STATE_IDLE;
        break;
    case GAPROLE_ERROR:
        HalLcdWriteString("Peripheral Error",HAL_LCD_LINE_1);
        break;
    default:
        HalLcdWriteString("",HAL_LCD_LINE_1);
        break;
    }
}

void callback_charactersitic_value(uint8 paramID)//peripheral
{
    uint8 newValue,value[SIMPLEPROFILE_CHAR6_LEN];
    uint8 length;

    switch(paramID)
    {
    case SIMPLEPROFILE_CHAR1:
        SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR1,&newValue,&length);
        HalLcdWriteStringValue("Peripheral Char1:",(uint16)newValue,10,HAL_LCD_LINE_2);
        break;
    case SIMPLEPROFILE_CHAR3:
        SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3,&newValue,&length);
        HalLcdWriteStringValue("Peripheral Char3:",(uint16)newValue,10,HAL_LCD_LINE_2);
        break;
    case SIMPLEPROFILE_CHAR6:
        SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR6,value,&length);
        if (length>0)
        {
            NPI_WriteTransport(value,length);
            HalLcdWriteString((char *)value,HAL_LCD_LINE_2);
        }
        cmd_parse(value,length,g_taskid);
        break;
    default:
        //should not reach here!
        break;
    }
}

void callback_rssi(uint16 connHandle,int8 rssi)
{
    HalLcdWriteStringValue("RSSI -dB:",(uint8)(-rssi),10,HAL_LCD_LINE_1);
}

uint8 callback_event(gapRoleEvent_t *pEvent)//central
{
    uint8 i;
    uint8 filter_uuid=TRUE;//TRUE-filter discovery results on desired service UUID,FALSE-not filter discovery results
    uint8 buffer[19];//3(preindex)+2(0x)+12(address)+2(\r\n)
    uint8 uuid[ATT_BT_UUID_SIZE];
    attWriteReq_t att_req;
    multi_t *central;
    uint8 bluetooth,attach;
    bool flag;
    
    switch(pEvent->gap.opcode)
    {
    case GAP_DEVICE_INIT_DONE_EVENT://once
        g_init=TRUE;
        HalLcdWriteString("Central Init",HAL_LCD_LINE_1);
        HalLcdWriteString(address2string(pEvent->initDone.devAddr),HAL_LCD_LINE_2);
        //central auto(init->scan)
        osal_set_event(g_taskid,APP_EVENT_SCAN);//osal_start_timerEx(g_taskid,APP_EVENT_SCAN,DELAY_GAP);
        break;
    case GAP_DEVICE_INFO_EVENT:
        if (filter_uuid==TRUE)
        {
            if (find_uuid(SIMPLEPROFILE_SERV_UUID,pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen))
            {
                //Add a device to the device discovery result list
                if (g_scannum<SCAN_MAXNUM)//If result count not at max
                {
                    //Check if device is already in scan results
                    for (i=0;i<g_scannum;i++)
                        if (osal_memcmp(pEvent->deviceInfo.addr,g_scanlist[i].addr,B_ADDR_LEN))
                            goto next;
                    //Check if device is already in multi_central
                    if (multi_check(pEvent->deviceInfo.addr))
                        goto next;
                    //Check if device is >BLUETOOTH_V40(m+s)
                    flag=parse_manufacture(&bluetooth,&attach,pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen,ID_NONE);
                    if (!flag)
                        flag=parse_manufacture(&bluetooth,&attach,pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen,ID_NET);
                    if (bluetooth<=BLUETOOTH_V40)
                        goto next;
                    /*
#if VERSION==VERSION_DEMO
                    if (g_fire)
                    {
                        if (!find_manufacture(MANUFACTURE_ID,g_manufacture[0],pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen))
                            goto next;
                        osal_memcpy(g_phoneaddr,pEvent->deviceInfo.addr,B_ADDR_LEN);
                    }
                    if (g_m2m)
                    {
                        if (!osal_memcmp(pEvent->deviceInfo.addr,g_addr[DEVICE_CAMERA],B_ADDR_LEN) && 
                            !osal_memcmp(pEvent->deviceInfo.addr,g_addr[DEVICE_AIRCONDITIONER],B_ADDR_LEN) &&
                            !find_manufacture(MANUFACTURE_ID,g_manufacture[0],pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen))
                            goto next;
                    }
//#elif VERSION==VERSION_MODULE && DEBUG==DEBUG_BOTH
//                    if (!find_manufacture(MANUFACTURE_ID,g_manufacture[0],pEvent->deviceInfo.pEvtData,pEvent->deviceInfo.dataLen))
//                        goto next;
#endif*/
                    //Add addr to scan result list
                    osal_memcpy(g_scanlist[g_scannum].addr,pEvent->deviceInfo.addr,B_ADDR_LEN);
                    g_scanlist[g_scannum].addrType=pEvent->deviceInfo.addrType;
                    g_scannum++;
                }
            }
        }
next:
        break;
    case GAP_DEVICE_DISCOVERY_EVENT:
        g_scan=FALSE;//discovery complete
        if (filter_uuid==FALSE)
        {
            //Copy results
            g_scannum=pEvent->discCmpl.numDevs;
            osal_memcpy(g_scanlist,pEvent->discCmpl.pDevList,sizeof(gapDevRec_t)*pEvent->discCmpl.numDevs);
        }
        //
        HalLcdWriteStringValue("Central Device",g_scannum,10,HAL_LCD_LINE_1);
        if (!g_scannum)
        //{
            //central auto(->init)
            //osal_set_event(g_taskid,APP_EVENT_INITIAL);//osal_start_timerEx(g_taskid,APP_EVENT_START,DELAY_GAP);
            break;
        //}
        //npi
        for (i=0;i<g_scannum;i++)
        {
            buffer[0]='[';
            buffer[1]=i+'0';
            buffer[2]=']';
            osal_memcpy(&buffer[3],address2string(g_scanlist[i].addr),B_ADDR_LEN*2+2);
            buffer[17]=0x0d;
            buffer[18]=0x0a;
            NPI_WriteTransport(buffer,19);
        }
        //central auto(scan->connect)
        osal_set_event(g_taskid,APP_EVENT_CONNECT);//osal_start_timerEx(g_taskid,APP_EVENT_CONNECT,DELAY_GAP);
        break;
    case GAP_LINK_ESTABLISHED_EVENT:
        //central=multi_point(pEvent->linkCmpl.connectionHandle);
        if (pEvent->gap.hdr.status==SUCCESS)
        {
            //state_ble=STATUS_CONNECT;
            //handle_connect=pEvent->linkCmpl.connectionHandle;
            uuid[0]=LO_UINT16(SIMPLEPROFILE_SERV_UUID);
            uuid[1]=HI_UINT16(SIMPLEPROFILE_SERV_UUID);
            GATT_DiscPrimaryServiceByUUID(pEvent->linkCmpl.connectionHandle,uuid,ATT_BT_UUID_SIZE,g_taskid);//discovery simple BLE service
            HalLcdWriteString("Central Connect",HAL_LCD_LINE_1);
            HalLcdWriteString(address2string(pEvent->linkCmpl.devAddr),HAL_LCD_LINE_2);
            //g_timeout=FALSE;
            //npi
            osal_memcpy(buffer,address2string(pEvent->linkCmpl.devAddr),B_ADDR_LEN*2+2);
            buffer[14]=0x0d;
            buffer[15]=0x0a;
            NPI_WriteTransport(buffer,16);
            //add into multi
            if (g_adv)//peripheral
            {
            }
            else//central
            {
                if (g_multi==NUM_CENTRAL)
                    break;
                g_central[g_multi].handle_conn=pEvent->linkCmpl.connectionHandle;//initial GAP_CONNHANDLE_INIT
                g_central[g_multi].handle_char=NULL;
                g_central[g_multi].latency=pEvent->linkCmpl.connLatency;
                g_central[g_multi].timeout=pEvent->linkCmpl.connTimeout;
                g_central[g_multi].interval=pEvent->linkCmpl.connInterval;
                osal_memcpy(g_central[g_multi].addr,pEvent->linkCmpl.devAddr,B_ADDR_LEN);
                g_central[g_multi].status=STATE_CONNECTED;
                g_multi++;
            }
        }
        else
        {
            //state_ble=BLE_STATE_IDLE;
            //state_discovery=BLE_DISC_STATE_IDLE;
            //handle_char=0;
            //handle_connect=GAP_CONNHANDLE_INIT;
            HalLcdWriteStringValue("Connect Failed:",pEvent->gap.hdr.status,10,HAL_LCD_LINE_1);
        }
        break;
    case GAP_LINK_TERMINATED_EVENT:
        HalLcdWriteStringValue("Central Disconn:",pEvent->linkTerminate.reason,10,HAL_LCD_LINE_1);
//        if (pEvent->linkTerminate.reason!=LL_STATUS_SUCCESS)
//        {
            //HalLcdWriteString("Central Init",HAL_LCD_LINE_1);
            //HalLcdWriteString(address2string(pEvent->initDone.devAddr),HAL_LCD_LINE_2);
            //central auto(->init)
//#if VERSION==VERSION_MODULE && DEBUG==DEBUG_BOTH
//            osal_start_timerEx(g_taskid,APP_EVENT_INITIAL,DELAY_EMERGENCY);
//#else
            //osal_set_event(g_taskid,APP_EVENT_INITIAL);//osal_start_timerEx(g_taskid,APP_EVENT_START,DELAY_GAP);
//#endif
//        }
        //del into multi
        //if (g_role==ROLE_PERIPHERAL)
        if (g_adv)//peripheral
        {
        }
        else//central
        {
            central=multi_point(pEvent->linkTerminate.connectionHandle);
            central->status=STATE_IDLE;
            multi_del();
        }
        break;
    case GAP_LINK_PARAM_UPDATE_EVENT:
        central=multi_point(pEvent->linkUpdate.connectionHandle);
        HalLcdWriteString("Central Para",HAL_LCD_LINE_1);
        att_req.handle=central->handle_char+1;//0x0027
        att_req.len=2;
        att_req.sig=0;
        att_req.cmd=0;
        att_req.pValue=GATT_bm_alloc(pEvent->linkUpdate.connectionHandle,ATT_WRITE_REQ,2,NULL);
        att_req.pValue[0]=0x01;
        att_req.pValue[1]=0x00;
        GATT_WriteCharValue(0,&att_req,g_taskid);
        HalLcdWriteStringValue("Central Notice",att_req.handle,10,HAL_LCD_LINE_1);
        break;
    default:
        break;
    }
    
    return TRUE;
}

void callback_passcode(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs)//central/peripheral
{
    uint32 passcode;
    uint8 str[7];

    LL_Rand((uint8 *)&passcode,sizeof(uint32));//Create random passcode
    passcode%=1000000;
    //Display passcode to user
    if (uiOutputs)
    {
        HalLcdWriteString("Passcode:",HAL_LCD_LINE_1);
        HalLcdWriteString((char *)_ltoa(passcode,str,10),HAL_LCD_LINE_2);
    }
    GAPBondMgr_PasscodeRsp(connectionHandle,SUCCESS,passcode);//Send passcode response
}

void callback_pairstate(uint16 connHandle,uint8 state,uint8 status)//central/peripheral
{
    //Pairing state
    multi_t *central;
    
    switch(state)
    {
    case GAPBOND_PAIRING_STATE_STARTED:
        HalLcdWriteString("Pairing Started",HAL_LCD_LINE_6);
        break;
    case GAPBOND_PAIRING_STATE_COMPLETE:
        if (status==SUCCESS)
            HalLcdWriteString("Pairing Success",HAL_LCD_LINE_7);
        else
        {
            HalLcdWriteStringValue("Pairing Failed",status,10,HAL_LCD_LINE_7);
            central=multi_point(connHandle);
            if (central->status==STATE_CONNECTED)
            {
                GAPRole_TerminateConnection(connHandle);//terminate link
                HAL_SYSTEM_RESET();//reset
            }
        }
        break;
    case GAPBOND_PAIRING_STATE_BONDED:
        if (status==SUCCESS)
            HalLcdWriteString("Bonding Success",HAL_LCD_LINE_8);
        break;
    }
}

//uart callback: use this callback as soon as recv data in uart. this callback will frequency used without NPI_ReadTransport
//we use time interval to handle the data section, as enough data or overtime occurs, parsing the section later with AT command.
void callback_npi(uint8 port,uint8 events)
{
    (void)port;//(void) use to avoid compile warnings, so ignore this var
    uint8 *buffer;//buffer point
    uint8 number;//buffer bytes
    static uint32 time[2],flag=0;

    if (events & (HAL_UART_RX_TIMEOUT | HAL_UART_RX_FULL))//have data in uart
    {
        number=0;
        number=NPI_RxBufLen();//get bytes in buffer
        if (!number)
            return;
        if (!flag)
        {
            flag=1;
            time[0]=time[1]=osal_GetSystemClock();
        }
        else
            time[1]=osal_GetSystemClock();
        if (number>=SIMPLEPROFILE_CHAR6_LEN || time[1]-time[0]>20)//if length>=SIMPLEPROFILE_CHAR6_LEN or intervals>20ms
        {
            flag=0;
            buffer=osal_mem_alloc(number);
            if (!buffer)
                return;
            //npi r/w and parsing
            NPI_ReadTransport(buffer,number);//read uart's data and release
            npi_parse(buffer,number,g_taskid);
            osal_mem_free(buffer);
        }
    }
}
