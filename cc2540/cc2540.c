#include "cc2540.h"

//app
uint8 g_npi;//NPI_NONE-transparent parsing,NPI_AT-at parsing
uint8 g_taskid;//Task ID for internal task/event processing
bool g_init;//TRUE-central init finished,FALSE-central init unfinished
//bool g_timeout;//TRUE-timeout,FALSE-timein
bool g_turnover;//TRUE-turn over between central/peripheral,FALSE-none
bool g_adv;//TRUE-adv enable,FALSE-adv disable
bool g_scan;//TRUE-scan start,FALSE-scan stop
uint8 g_scannum;//scan found number
gapDevRec_t g_scanlist[SCAN_MAXNUM];//scan list
multi_t g_central[NUM_CENTRAL];//central array
multi_t g_peripheral;//peripheral array
uint8 g_multi;//central array number
//bool g_switch;//SWITCH_ON-on,SWITCH_OFF-off
//bool g_fire;//TRUE-fire alarm,FALSE-none
    
//Process an incoming sys task message
static void Sys_ProcessEvent(osal_event_hdr_t *pMsg)
{
    attReadByTypeReq_t req;
    gattMsgEvent_t *p_gattmsgevent;
    multi_t *central;
    static uint16 handle_service[2];//discovered service start/end handle
    static uint8 number=0;//write bytes

    switch(pMsg->event)
    {
    case GATT_MSG_EVENT:
        p_gattmsgevent=(gattMsgEvent_t *)pMsg;
        central=multi_point(p_gattmsgevent->connHandle);
        if (central->status!=STATE_CONNECTED && central->status!=STATE_SERVICES && central->status!=STATE_CHARACTERISTIC)//In case a GATT message came after a connection has dropped, ignore the message
            return;
        if (p_gattmsgevent->method==ATT_READ_RSP || (p_gattmsgevent->method==ATT_ERROR_RSP && p_gattmsgevent->msg.errorRsp.reqOpcode==ATT_READ_REQ))
        {
            if (p_gattmsgevent->method==ATT_ERROR_RSP)
                HalLcdWriteStringValue("Read Error",p_gattmsgevent->msg.errorRsp.errCode,10,HAL_LCD_LINE_1);
            else//read success
                HalLcdWriteStringValue("Read Rsp:",p_gattmsgevent->msg.readRsp.pValue[0],10,HAL_LCD_LINE_1);
        }
        else if (p_gattmsgevent->method==ATT_WRITE_RSP || (p_gattmsgevent->method==ATT_ERROR_RSP && p_gattmsgevent->msg.errorRsp.reqOpcode==ATT_WRITE_REQ))
        {
            if (p_gattmsgevent->method==ATT_ERROR_RSP)
                HalLcdWriteStringValue("Write Error",p_gattmsgevent->msg.errorRsp.errCode,10,HAL_LCD_LINE_1);
            else//write success
                HalLcdWriteStringValue("Write Sent:",number++,10,HAL_LCD_LINE_1);
        }
        else if (p_gattmsgevent->method==ATT_HANDLE_VALUE_NOTI)//notify
        {
            if (p_gattmsgevent->msg.handleValueNoti.handle==central->handle_char)//char6's notify
            {
                HalLcdWriteStringValue("Notify:",p_gattmsgevent->msg.handleValueNoti.pValue[0],16,HAL_LCD_LINE_1);
                NPI_WriteTransport(p_gattmsgevent->msg.handleValueNoti.pValue,p_gattmsgevent->msg.handleValueNoti.len);
            }
        }
        switch(central->status)
        {
        case STATE_CONNECTED:
            if (p_gattmsgevent->method==ATT_FIND_BY_TYPE_VALUE_RSP && p_gattmsgevent->msg.findByTypeValueRsp.numInfo>0)//Service found, store group's 2 handles
            {
                handle_service[0]=ATT_ATTR_HANDLE(p_gattmsgevent->msg.findByTypeValueRsp.pHandlesInfo,0);
                handle_service[1]=ATT_GRP_END_HANDLE(p_gattmsgevent->msg.findByTypeValueRsp.pHandlesInfo,0);
            }
            if ((p_gattmsgevent->method==ATT_FIND_BY_TYPE_VALUE_RSP && p_gattmsgevent->hdr.status==bleProcedureComplete) || p_gattmsgevent->method==ATT_ERROR_RSP)//If procedure complete
            {
                if (handle_service[0]!=0)//Discover characteristic
                {
                    central->status=STATE_SERVICES;
                    HalLcdWriteString("Central Service",HAL_LCD_LINE_1);
                    req.startHandle=handle_service[0];
                    req.endHandle=handle_service[1];
                    req.type.len=ATT_BT_UUID_SIZE;
                    req.type.uuid[0]=LO_UINT16(SIMPLEPROFILE_CHAR6_UUID);
                    req.type.uuid[1]=HI_UINT16(SIMPLEPROFILE_CHAR6_UUID);
                    GATT_ReadUsingCharUUID(central->handle_conn,&req,g_taskid);
                }
            }
            break;
        case STATE_SERVICES:
            if (p_gattmsgevent->method==ATT_READ_BY_TYPE_RSP && p_gattmsgevent->msg.readByTypeRsp.numPairs>0)//Characteristic found, store char handle
            {
                HalLcdWriteString("Central Char",HAL_LCD_LINE_1);
                central->handle_char=BUILD_UINT16(p_gattmsgevent->msg.readByTypeRsp.pDataList[0],p_gattmsgevent->msg.readByTypeRsp.pDataList[1]);
                central->status=STATE_CHARACTERISTIC;
                //central auto(connect->transfer)
                osal_start_timerEx(g_taskid,APP_EVENT_SWITCH,DELAY_GAP);//osal_set_event(g_taskid,APP_EVENT_TRANSFER);
            }
            break;
        }
        GATT_bm_free(&p_gattmsgevent->msg,p_gattmsgevent->method);
        break;
    }
}

void App_Init(uint8 task_id)
{
    uint8 i;
    uint16 j;
    uint32 k;
    uint8 gap_adtype_flags;
    uint8 gaprole_param_update_enable;
    uint16 gap_adtype_slave_conn_interval_range[2];
    uint16 manufacture_id;
#if COMPILE==COMPILE_ITC
    static uint8 adv_name[]="ITC";
#elif COMPILE==COMPILE_REFRIGERATOR
    static uint8 adv_name[]="Refrigerator";
#elif COMPILE==COMPILE_GATELOCK
    static uint8 adv_name[]="GateLock";
#elif COMPILE==COMPILE_CAMERA
    static uint8 adv_name[]="Camera";
#elif COMPILE==COMPILE_AIRCONDITION
    static uint8 adv_name[]="AirCondition";
#elif COMPILE==COMPILE_GARAGE
    static uint8 adv_name[]="Garage";
#elif COMPILE==COMPILE_CARLOCK
    static uint8 adv_name[]="CarLock";
#endif
    static uint8 simpleprofile_char5[SIMPLEPROFILE_CHAR5_LEN]={1,2,3,4,5};//simple profile char5
    static uint8 simpleprofile_char6[SIMPLEPROFILE_CHAR6_LEN]={6,2,3,4,5};//simple profile char6
    static uint8 table_scanrsp[31];//scan response data(max 31 bytes)
    static uint8 table_adv[31];//advertisement data(max 31 bytes,best kept short to conserve power)
    static simpleProfileCBs_t simpleprofile_cbs=
    {
        callback_charactersitic_value //Charactersitic value change callback
    };//Simple GATT Profile Callbacks
    static uint8 ggs_device_name_att[GAP_DEVICE_NAME_LEN]="ITC";//GAP GATT Attributes
    static gapBondCBs_t gap_bond_cbs=
    {
        callback_passcode,      //passcode callback(no display when use NULL)
        callback_pairstate      //pair state callback
    };//Bond Manager Callbacks

    //initial(var/led/npi/key)
#if DEBUG==DEBUG_C || DEBUG==DEBUG_P
    g_turnover=TRUE;
#endif
    g_npi=NPI_AT;
    g_taskid=task_id;//conserve task id to global var
    g_init=FALSE;
    g_scan=FALSE;
    g_multi=0;
#if DEBUG==DEBUG_CP || DEBUG==DEBUG_C
    g_adv=TRUE;
#else //DEBUG_PC || DEBUG_P
    g_adv=FALSE;
#endif
#if COMPILE==COMPILE_REFRIGERATOR || COMPILE==COMPILE_CARLOCK
    HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
    HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
    HalLedSet(HAL_LED_3,HAL_LED_MODE_ON);
    //g_switch=SWITCH_ON;
#else
    HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
    HalLedSet(HAL_LED_2,HAL_LED_MODE_OFF);
    HalLedSet(HAL_LED_3,HAL_LED_MODE_OFF);
    //g_switch=SWITCH_OFF;
#endif
    NPI_InitTransport(callback_npi);//default baudrate 115200(modify baud in npi.h)
    RegisterForKeys(g_taskid);
    //1.peripheral setting
    //setup the GAP
    GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL,5);//Connection Pause Peripheral time value(default 5s)
    //setup the GAP Role Parameters
    gap_adtype_flags=GAP_ADTYPE_FLAGS_GENERAL;//GAP_ADTYPE_FLAGS_LIMITED-advertises for 30.72s only,GAP_ADTYPE_FLAGS_GENERAL-advertises indefinitely
    gap_adtype_slave_conn_interval_range[0]=0x0006;//0x0050;//Minimum connection interval,1.25ms*80=100ms
    gap_adtype_slave_conn_interval_range[1]=0x0006;//0x0320;//Maximum connection interval,1.25ms*800=1000ms
    gaprole_param_update_enable=TRUE;//Slave Connection Parameter Automatic Update Enable.TRUE-enable,FALSE-disable
    manufacture_id=ID_NONE;
    i=sizeof(adv_name);
    table_scanrsp[0]=i+1;
    table_scanrsp[1]=GAP_ADTYPE_LOCAL_NAME_COMPLETE;//Complete local name
    osal_memcpy(&table_scanrsp[2],adv_name,i);
    table_scanrsp[i+2]=0x05;
    table_scanrsp[i+3]=GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE;//Min and Max values of the connection interval
    table_scanrsp[i+4]=LO_UINT16(gap_adtype_slave_conn_interval_range[0]);
    table_scanrsp[i+5]=HI_UINT16(gap_adtype_slave_conn_interval_range[0]);
    table_scanrsp[i+6]=LO_UINT16(gap_adtype_slave_conn_interval_range[1]);
    table_scanrsp[i+7]=HI_UINT16(gap_adtype_slave_conn_interval_range[1]);
    table_scanrsp[i+8]=0x02;
    table_scanrsp[i+9]=GAP_ADTYPE_POWER_LEVEL;//Tx Power level
    table_scanrsp[i+10]=0x00;//0dBm
    table_scanrsp[i+11]=0x05;//self define data:bluetooth version,with something
    table_scanrsp[i+12]=GAP_ADTYPE_MANUFACTURER_SPECIFIC;
    table_scanrsp[i+13]=LO_UINT16(manufacture_id);
    table_scanrsp[i+14]=HI_UINT16(manufacture_id);
    table_scanrsp[i+15]=BLUETOOTH_V40;
    table_scanrsp[i+16]=ATTACH_NONE;
    table_adv[0]=0x02;
    table_adv[1]=GAP_ADTYPE_FLAGS;//Advertise Type
    table_adv[2]=gap_adtype_flags | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
    table_adv[3]=0x03;
    table_adv[4]=GAP_ADTYPE_16BIT_MORE;//Some of the uuid data
    table_adv[5]=LO_UINT16(SIMPLEPROFILE_SERV_UUID);
    table_adv[6]=HI_UINT16(SIMPLEPROFILE_SERV_UUID);
    table_adv[7]=0x05;
    table_adv[8]=GAP_ADTYPE_MANUFACTURER_SPECIFIC;
    table_adv[9]=LO_UINT16(manufacture_id);
    table_adv[10]=HI_UINT16(manufacture_id);
    table_adv[11]=BLUETOOTH_V40;
    table_adv[12]=ATTACH_NONE;
    if (gap_adtype_flags==GAP_ADTYPE_FLAGS_LIMITED)
    {
        j=0x0000;
        GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME,sizeof(uint16),&j);//Advertising Off Time for Limited advertisements(default 30.72s)
    }
    i=31;
    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA,i,table_scanrsp);//Scan Response Data(default all 0)
    i=13;
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA,i,table_adv);//Advertisement Data(default 02:01:01)
    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE,sizeof(uint8),&gaprole_param_update_enable);//Slave Connection Parameter Automatic Update Enable
    if (gaprole_param_update_enable==TRUE)
    {
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL,sizeof(uint16),&gap_adtype_slave_conn_interval_range[0]);//Minimum Connection Interval to allow n*1.25ms[7.5ms,4s](default n=0x0006)
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL,sizeof(uint16),&gap_adtype_slave_conn_interval_range[1]);//Maximum Connection Interval to allow n*1.25ms[7.5ms,4s](default n=0x0c80)
        j=0x0000;
        GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY,sizeof(uint16),&j);//Update Parameter Slave Latency[0,499](default 0)
        j=0x0064;//0x03e8;
        GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER,sizeof(uint16),&j);//Update Parameter Timeout Multiplier n*10ms[100ms,32s](default n=0x3e8)
    }
    //setup the GAP Characteristics
    if (gap_adtype_flags==GAP_ADTYPE_FLAGS_LIMITED)
    {
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN,160);//Minimum advertising interval n*0.625ms
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX,160);//Maximum advertising interval n*0.625ms
    }
    else
    {
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN,160);//Minimum advertising interval n*0.625ms
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX,160);//Maximum advertising interval n*0.625ms
    }
    //setup GATT attributes
    DevInfo_AddService();//Initializes the Device Information service
    SimpleProfile_AddService(GATT_ALL_SERVICES);//Initializes the Simple GATT Profile service
#if defined FEATURE_OAD
    OADTarget_AddService();//OAD Profile
#endif
    //setup the SimpleProfile Characteristic Values
    i=1;
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1,sizeof(uint8),&i);//Profile Characteristic
    i=2;
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2,sizeof(uint8),&i);//Profile Characteristic
    i=3;
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3,sizeof(uint8),&i);//Profile Characteristic
    i=4;
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4,sizeof(uint8),&i);//Profile Characteristic
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5,SIMPLEPROFILE_CHAR5_LEN,simpleprofile_char5);//Profile Characteristic
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR6,SIMPLEPROFILE_CHAR6_LEN,simpleprofile_char6);//Profile Characteristic(self define)
    SimpleProfile_RegisterAppCBs(&simpleprofile_cbs);//register simpleprofile's callback
    HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT);//Enable clock divide on halt, This reduces active current while radio is active and CC254x MCU is halted
    HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_0_DBM);//set the transmit power
    //2.central setting
    //setup Central Profile
    i=SCAN_MAXNUM;
    GAPRole_SetParameter(GAPROLE_MAX_SCAN_RES,sizeof(uint8),&i);//Maximum number of scan responses(default 0-unlimited)
    //setup GAP
    GAP_SetParamValue(TGAP_GEN_DISC_SCAN,4000);//Minimum time to perform scanning in General Discovery proc(ms)
    GAP_SetParamValue(TGAP_LIM_DISC_SCAN,4000);//Minimum time to perform scanning in Limited Discovery proc(ms)
    //setup GATT attributes
    GATT_InitClient();//initialize GATT Client
    GATT_RegisterForInd(g_taskid);//Register to receive incoming ATT Indications/Notifications
    //3.common setting
    //setup the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT,GAP_DEVICE_NAME_LEN,ggs_device_name_att);
    //setup the GAP Bond Manager
    k=0;
    GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE,sizeof(uint32),&k);//The default passcode for MITM protection[0,999999](default 0)
    i=GAPBOND_PAIRING_MODE_NO_PAIRING;//v1.9 use GAPBOND_PAIRING_MODE_INITIATE;
    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE,sizeof(uint8),&i);//Pairing Mode.GAPBOND_PAIRING_MODE_NO_PAIRING-pairing is not allowed,GAPBOND_PAIRING_MODE_WAIT_FOR_REQ-Wait for a pairing request or slave security request,GAPBOND_PAIRING_MODE_INITIATE-Don't wait, initiate a pairing request or slave security request
    i=FALSE;//v1.9 use TRUE
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION,sizeof(uint8),&i);//Man-In-The-Middle (MITM) basically turns on Passkey protection in the pairing algorithm(default:FALSE-disabled,TRUE-enabled)
    i=GAPBOND_IO_CAP_DISPLAY_ONLY;
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES,sizeof(uint8),&i);//I/O capabilities(default GAPBOND_IO_CAP_DISPLAY_ONLY-display & capture input,GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT-no display & no input)
    i=TRUE;
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED,sizeof(uint8),&i);//Request Bonding during the pairing process if enabled(default:FALSE-disabled and bonding for each connection,TRUE-enabled and bonding for only once)
    //setup GATT attributes
    GGS_AddService(GATT_ALL_SERVICES);//Add function for the GAP GATT Service
    GATTServApp_AddService(GATT_ALL_SERVICES);//Add function for the GATT Service
    GAPBondMgr_Register((gapBondCBs_t *)&gap_bond_cbs);//Start Bond Manager
    //set an event using osal method
    osal_set_event(g_taskid,APP_EVENT_SWITCH);
}

uint16 App_ProcessEvent(uint8 task_id,uint16 events)
{
    VOID task_id;//OSAL required parameter that isn't used in this function
    uint8 *pMsg;
    static gapRoleCB_t gap_roles_cbs=
    {
        callback_profile_state, //Profile State Change Callbacks
        NULL,                   //When a valid RSSI is read from controller (not used by application)
        callback_rssi,          //RSSI callback
        callback_event          //Event callback
    };//GAP Role Callbacks
    //static uint8 value='A';
    //uint8 buffer[2];
//#if DEBUG==DEBUG_P || DEBUG==DEBUG_C
//    static bool once=TRUE;
//#endif
    uint8 i;
    uint8 buffer[20];
    //bool flag;
    
    //system event msg(key or ble r/w)
    if (events & SYS_EVENT_MSG)
    {
        pMsg=osal_msg_receive(g_taskid);
        if (pMsg!=NULL)
        {
            Sys_ProcessEvent((osal_event_hdr_t *)pMsg);
            osal_msg_deallocate(pMsg);//Release the OSAL message
        }
        // return unprocessed events
        return events^SYS_EVENT_MSG;
    }
    //self defined event(totally 16 event by bits)
    if (events & APP_EVENT_SWITCH)//central/peripheral
    {
        osal_start_timerEx(g_taskid,APP_EVENT_SWITCH,DELAY_WAIT);
        //check central status
        if (!g_adv)
        {
            for (i=0;i<g_multi;i++)
            {
                if (g_central[i].status!=STATE_CHARACTERISTIC)
                    continue;
                buffer[0]=CMD_DATA;
                send_data(buffer,1,&g_central[i],g_taskid);
                NPI_WriteTransport(buffer,1);
                GAPRole_TerminateConnection(g_central[i].handle_conn);//Terminate a link(host)
            }
        }
        //check peripheral status
        if (g_adv && multi_connect(ROLE_PERIPHERAL,NULL))
        {
            buffer[0]=RET_DATA;
            send_data(buffer,1,&g_peripheral,g_taskid);
            NPI_WriteTransport(buffer,1);
        }
        //adv switch
#if DEBUG==DEBUG_P || DEBUG==DEBUG_C
        if (g_turnover)
        {
            //g_turnover=FALSE;
            //once=TRUE;
#endif
        g_adv=!g_adv;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED,sizeof(uint8),&g_adv);
#if DEBUG==DEBUG_P || DEBUG==DEBUG_C
        }
#endif
        //central judgement
        if (!g_adv && multi_num(ROLE_CENTRAL)==NUM_CENTRAL)//master exceed maximum
            return events^APP_EVENT_SWITCH;
        //peripheral judgement
        if (g_adv && multi_num(ROLE_PERIPHERAL)==NUM_PERIPHERAL)//peripheral exceed maximum
            return events^APP_EVENT_SWITCH;
        //switch->initial
        osal_set_event(g_taskid,APP_EVENT_INITIAL);//for adv enable event
        
        return events^APP_EVENT_SWITCH;
    }
    if (events & APP_EVENT_INITIAL)//central/peripheral
    {
        //start device
#if DEBUG==DEBUG_P || DEBUG==DEBUG_C
        if (g_turnover)
        {
            g_turnover=FALSE;
#endif
        if (g_adv)
        {
            //set_role(GAP_PROFILE_PERIPHERAL);
            GAPRole_Init(8,GAP_PROFILE_PERIPHERAL);
            GAPRole_StartDevice(&gap_roles_cbs);//Start the Device(central/peripheral)
            NPI_WriteTransport("Init:Open Adv\r\n",12);
            HalLcdWriteString("Init:Open Adv",HAL_LCD_LINE_1);
            //peripheral auto(->init)
            //osal_start_timerEx(g_taskid,APP_EVENT_START,DELAY_ADV);
        }
        else
        {
            //set_role(GAP_PROFILE_CENTRAL);
            GAPRole_Init(8,GAP_PROFILE_CENTRAL);
            GAPRole_StartDevice(&gap_roles_cbs);//Start the Device(central/peripheral)
            NPI_WriteTransport("Init:Close Adv\r\n",12);
            HalLcdWriteString("Init:Close Adv",HAL_LCD_LINE_1);
        }
#if DEBUG==DEBUG_P || DEBUG==DEBUG_C
        }
#endif
        //goto scan(central)
        //if (!g_adv && g_init)
        //if (g_init)
        if (!g_adv)
            osal_set_event(g_taskid,APP_EVENT_SCAN);//osal_start_timerEx(g_taskid,APP_EVENT_SCAN,DELAY_GAP);
        
        return events^APP_EVENT_INITIAL;
    }
    else if (events & APP_EVENT_SCAN)//central
    {
        multi_del();
        npi_parse("AT+SCAN\r\n",9,g_taskid);
        return events^APP_EVENT_SCAN;
    }
    else if (events & APP_EVENT_CONNECT)//central
    {
        for (i=0;i<g_scannum;i++)
        {
            osal_memcpy(buffer,"AT+CONNECT0\r\n",13);
            buffer[10]='0'+i;
            npi_parse(buffer,13,g_taskid);
        }
        return events^APP_EVENT_CONNECT;
    }
#if 0
    else if (events & APP_EVENT_TRANSFER)//central
    {
        for (i=0;i<g_multi;i++)
        {
            if (g_central[i].status!=STATE_CHARACTERISTIC)
                continue;
            send_data(&value,1,&g_central[i],g_taskid);
            NPI_WriteTransport(&value,1);
            if (value=='C')
            {
                value='A';
                //GAPRole_TerminateConnection(g_central[i].handle_conn);//Terminate a link(host)
                //osal_start_timerEx(g_taskid,APP_EVENT_INITIAL,DELAY_GAP);//osal_set_event(g_taskid,APP_EVENT_INITIAL);maybe delete
            }
            else
            {
                value++;
                osal_start_timerEx(g_taskid,APP_EVENT_TRANSFER,DELAY_GAP);
            }/*
#else
            if (multi_connect(ROLE_CENTRAL,g_central[i].handle_conn))
            {
                buffer[0]=RET_EMERGENCY;
                buffer[1]=(uint8)g_switch;
                send_data(buffer,2,&g_central[i],g_taskid);
                GAPRole_TerminateConnection(g_central[i].handle_conn);//Terminate a link(host)
                //osal_start_timerEx(g_taskid,APP_EVENT_TRANSFER,DELAY_EMERGENCY);
            }
#endif*/
            /*
#elif VERSION==VERSION_DEMO
            if (g_m2m)
            {                
                if (!osal_memcmp(g_addr[DEVICE_CAMERA],g_central[i].addr,B_ADDR_LEN) && !osal_memcmp(g_addr[DEVICE_AIRCONDITIONER],g_central[i].addr,B_ADDR_LEN))
                    continue;
                buffer[0]=CMD_SWITCH;
                buffer[1]=SWITCH_ON;
                send_data(buffer,2,&g_central[i],g_taskid);
                GAPRole_TerminateConnection(g_central[i].handle_conn);//Terminate a link(host)
            }
            if (g_fire)
            {
                i=multi_find(g_phoneaddr);
                if (i==NUM_CENTRAL)
                    continue;
                buffer[0]=RET_FIRE;
                send_data(buffer,1,&g_central[i],g_taskid);
                GAPRole_TerminateConnection(g_central[i].handle_conn);//Terminate a link(host)
            }
            //osal_start_timerEx(g_taskid,APP_EVENT_INITIAL,DELAY_GAP);//osal_set_event(g_taskid,APP_EVENT_INITIAL);maybe delete
#endif*/
        }
        /*
#if VERSION==VERSION_DEMO
        if (g_m2m)
        {
            buffer[0]=RET_M2M;
            send_data(buffer,1,&g_peripheral,g_taskid);
            GAPRole_TerminateConnection(g_peripheral.handle_conn);//Terminate a link(host)
            g_turnover=TRUE;
            g_m2m=FALSE;
        }
#endif*/
        return events^APP_EVENT_TRANSFER;
    }
#endif
    /*
    else if (events & APP_EVENT_TIMEOUT)//central
    {
        if (g_timeout==TRUE)
        {
            GAPRole_TerminateConnection(handle_connect);//Terminate a link
            osal_start_timerEx(g_taskid,APP_EVENT_INITIAL,DELAY_GAP);//osal_set_event(g_taskid,APP_EVENT_INITIAL);
        }
        return events^APP_EVENT_TIMEOUT;
    }*/
    /*
#if VERSION==VERSION_MODULE
#if DEBUG==DEBUG_BOTH
    else if (events & APP_EVENT_CLOCK)
    {
        if (multi_connect(ROLE_PERIPHERAL,NULL))
        {
            buffer[0]=RET_CLOCK;
            buffer[1]=(uint8)g_switch;
            send_data(buffer,2,&g_peripheral,g_taskid);
            osal_start_timerEx(g_taskid,APP_EVENT_CLOCK,DELAY_CLOCK);
        }
        return events^APP_EVENT_CLOCK;
    }
#endif
#endif
#if VERSION==VERSION_DEMO
    else if (events & APP_EVENT_TEMPERATURE)
    {
        static bool once=FALSE;
        
        if (!once && math_mean()>VALVE_TEMPERATURE)
        {
            once=TRUE;
            g_fire=TRUE;
            //g_turnover=TRUE;//peripheral->central
            //peripheral auto(->start)
            //osal_set_event(task_id,APP_EVENT_INITIAL);
        }
        else
            osal_start_timerEx(g_taskid,APP_EVENT_TEMPERATURE,DELAY_GAP);
        return events^APP_EVENT_TEMPERATURE;
    }
#endif*/
    //Discard unknown events
    return 0;
}

//The order in this table must be identical to the task initialization calls below in osalInitTask.
//Priority from high->low for each tasks
const pTaskEventHandlerFn tasksArr[] =
{
    LL_ProcessEvent,                                                  // task 0,in lib
    Hal_ProcessEvent,                                                 // task 1,in hal_drivers.c
    HCI_ProcessEvent,                                                 // task 2,in lib
#if defined (OSAL_CBTIMER_NUM_TASKS)
    OSAL_CBTIMER_PROCESS_EVENT(osal_CbTimerProcessEvent),             // task 3,in osal_cbtimer.c
#endif
    L2CAP_ProcessEvent,                                               // task 4,in lib
    GAP_ProcessEvent,                                                 // task 5,in lib
    GATT_ProcessEvent,                                                // task 6,in lib
    SM_ProcessEvent,                                                  // task 7,in lib
    GAPRole_ProcessEvent,                                             // task 8,in peripheral/central.c
    GAPBondMgr_ProcessEvent,                                          // task 9,in gapbondmgr.c
    GATTServApp_ProcessEvent,                                         // task 10,in lib
    //
    App_ProcessEvent                                                // task 11,self define
};
const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *tasksEvents;

void osalInitTasks( void )
{
    uint8 taskID=0;
    
    tasksEvents=(uint16 *)osal_mem_alloc(sizeof(uint16)*tasksCnt);
    osal_memset(tasksEvents,0,sizeof(uint16)*tasksCnt);
    //LL Task
    LL_Init(taskID++);
    //Hal Task
    Hal_Init(taskID++);
    //HCI Task
    HCI_Init(taskID++);
#if defined (OSAL_CBTIMER_NUM_TASKS)
    //Callback Timer Tasks
    osal_CbTimerInit(taskID);
    taskID+=OSAL_CBTIMER_NUM_TASKS;
#endif
    //L2CAP Task
    L2CAP_Init(taskID++);
    //GAP Task
    GAP_Init(taskID++);
    //GATT Task
    GATT_Init(taskID++);
    //SM Task
    SM_Init(taskID++);
    //Profiles
    GAPRole_Init(taskID,GAP_PROFILE_PERIPHERAL);
    GAPRole_Init(taskID,GAP_PROFILE_CENTRAL);
    taskID++;
    GAPBondMgr_Init(taskID++);
    GATTServApp_Init(taskID++);
    //Application
    App_Init(taskID);
}

//If device want to send out info, it should be act as a central to connect a peripheral.
//After finish the data transfer(central->peripheral or peripheral->central), then disconnect.
//Adv all the time, occasionally scan usable devices as a pulse.
void main(void)
{
    //Initialize hardware
    HAL_BOARD_INIT();//initial/stablelizate osc, open flash cache prefetch
    //Initialize board I/O
    InitBoard(OB_COLD);//OB_COLD:close led/interrupt(avoid interfere)
    //Initialze the HAL driver
    HalDriverInit();//initial timer/adc/dma/aes/lcd/led/uart/key/hid
    //Initialize NV system
    osal_snv_init();//snv:flash to conserve pair data or user define data(4KB)
    //Initialize LL
    //Initialize the operating system
    osal_init_system();//osal initial including memory malloc, info queue, timer, power management and sys task
    //Enable interrupts
    HAL_ENABLE_INTERRUPTS();//enable global interrupt
    //Final board initialization
    InitBoard(OB_READY);//initial key
#if defined (POWER_SAVING)
    //option 1: always in PM1
    //osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
    //option 2: enter PM2 when in idle
    //osal_pwrmgr_device( PWRMGR_BATTERY );
    //option 3: enter PM3 to disconnect ble, stop broadcasting, disable all timers. It waken up only by outer interrupt
    osal_pwrmgr_device(PWRMGR_BATTERY);//low power option
#endif
    //Start OSAL: looping to check flag in each tasks and each events, assign specific proc initialed in osal_init_system
    osal_start_system();//No Return from here
}