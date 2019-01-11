#include "include.h"

//app
extern uint8 g_npi;//NPI_NONE-transparent parsing,NPI_AT-at parsing
extern bool g_turnover;//TRUE-central,FALSE-peripheral
extern bool g_adv;//TRUE-adv enable,FALSE-adv disable
extern bool g_scan;//TRUE-scan start,FALSE-scan stop
extern uint8 g_scannum;//scan found number
extern gapDevRec_t g_scanlist[SCAN_MAXNUM];//scan list
extern multi_t g_central[NUM_CENTRAL];//central array
extern multi_t g_peripheral;//peripheral array
extern uint8 g_multi;//central array number
//extern bool g_switch;//SWITCH_ON-on,SWITCH_OFF-off

//delay in ms(sys clock default 16MHZ)
void delay(uint16 msec)
{
    uint16 i,j;
    
    for (i=0;i<msec;i++)
        for (j=0;j<535;j++);
}

//0-not identical,1-identical
uint8 str_cmp(uint8 *p1,uint8 *p2,uint8 len)
{
    uint8 i=0;
    while(i<len)
    {
        if (p1[i]!=p2[i])
            return 0;
        i++;
    }
    
    return 1;
}

//Convert Bluetooth address into string
char *address2string(uint8 *pAddr)
{
    uint8 i;
    char hex[]="0123456789ABCDEF";
    static char str[15];
    char *pStr=str;

    *pStr++='0';
    *pStr++='x';
    //Start from end of addr
    pAddr+=B_ADDR_LEN;
    for (i=B_ADDR_LEN;i>0;i--)
    {
        *pStr++=hex[*--pAddr>>4];
        *pStr++=hex[*pAddr & 0x0f];
    }
    *pStr=0;

    return str;
}

//Find a given Manufacture info in an advertiser's adv
//TRUE-find,FALSE-not find
bool find_manufacture(uint16 id,uint8 *manufacture,uint8 *content,uint8 length)
{
    //according to len-tag-value stucture
    uint8 i;
    uint8 len,type;
    uint8 *end;

    end=content+length-1;
    while(content<end)//While end of data not reached
    {
        //Get length of next AD item
        len=*content++;
        if (!len)
            continue;
        type=*content;
        if (type==GAP_ADTYPE_MANUFACTURER_SPECIFIC)//If AD type is for Manufacture info
        {
            content++;
            len--;
            //Check length
            if (len!=sizeof(uint16)+MANUFACTURE_LEN)
                goto next;
            //Check manufacture id
            if (content[0]!=LO_UINT16(id) || content[1]!=HI_UINT16(id))
                goto next;
            //Check manufacture data
            for (i=0;i<MANUFACTURE_LEN;i++)
                if (content[2+i]!=manufacture[i])
                    break;
            return TRUE;
        }
next:
        content+=len;
    }
    
    return FALSE;//Match not found    
}

//Find a given UUID in an advertiser's service UUID list
//TRUE-find,FALSE-not find
bool find_uuid(uint16 uuid,uint8 *content,uint8 length)
{
    uint8 len,type;
    uint8 *end;

    end=content+length-1;
    while(content<end)//While end of data not reached
    {
        //Get length of next AD item
        len=*content++;
        if (!len)
            continue;
        type=*content;
        if (type==GAP_ADTYPE_16BIT_MORE || type==GAP_ADTYPE_16BIT_COMPLETE)//If AD type is for 16-bit service UUID
        {
            content++;
            len--;
            //For each UUID in list
            while(len>=2 && content<end)
            {
                //Check for match
                if (content[0]==LO_UINT16(uuid) && content[1]==HI_UINT16(uuid))//Match found
                    return TRUE;
                //Go to next 2 bytes
                content+=2;
                len-=2;
            }
            //Handle possible erroneous extra byte in UUID list
            if (len==1)
                content++;
        }
        else//Go to next item
            content+=len;
    }
    
    return FALSE;//Match not found
}

void send_data(uint8 *buffer,uint8 number,multi_t *multi,uint8 task_id)
{
    static attHandleValueNoti_t att_noti;
    attWriteReq_t att_req;
    
    if (g_adv)//peripheral
    {
        att_noti.len=number;
        att_noti.handle=0x0026;
        att_noti.pValue=GATT_bm_alloc(multi->handle_conn,ATT_WRITE_REQ,number,NULL);
        osal_memcpy(att_noti.pValue,buffer,number);
        GATT_Notification(multi->handle_conn,&att_noti,FALSE);
    }
    else//central
    {
        att_req.handle=multi->handle_char;
        att_req.len=number;
        att_req.sig=0;
        att_req.cmd=0;
        att_req.pValue=GATT_bm_alloc(multi->handle_conn,ATT_WRITE_REQ,number,NULL);
        osal_memcpy(att_req.pValue,buffer,number);
        GATT_WriteCharValue(multi->handle_conn,&att_req,task_id);
    }
}

bool parse_manufacture(uint8 *bluetooth,uint8 *attach,uint8 *manufacture,uint8 length,uint16 id)
{
    //according to len-tag-value stucture
    uint8 len;
    uint8 *end;

    end=manufacture+length-1;
    while(manufacture<end)//While end of data not reached
    {
        //Get length of next AD item
        len=*manufacture++;
        if (!len)
            continue;
        if (*manufacture!=GAP_ADTYPE_MANUFACTURER_SPECIFIC)//If AD type is for Manufacture info
        {
            manufacture+=len;
            continue;
        }
        manufacture++;
        len--;
        //Check length
        if (len!=sizeof(uint16)+MANUFACTURE_LEN)
        {
            manufacture+=len;
            continue;
        }
        if (manufacture[0]!=LO_UINT16(id) || manufacture[1]!=HI_UINT16(id))
        {
            manufacture+=len;
            continue;
        }
        *bluetooth=manufacture[2];
        *attach=manufacture[3];
        return TRUE;
    }
    
    return FALSE;
}

void npi_parse(uint8 *buffer,uint8 number,uint8 task_id)
{
    if (g_npi==NPI_NONE)
    {
        NPI_WriteTransport(buffer,number);
        return;
    }
    if (number==4 && str_cmp(buffer,"AT\r\n",number))//AT. for test
        NPI_WriteTransport("OK",2);
    else if (number==9 && str_cmp(buffer,"AT+NPI",6))//AT+NPI?. for npi change
    {
        g_npi=buffer[6]-'0';
        NPI_WriteTransport(&buffer[3],6);
    }
    else if (number==9 && str_cmp(buffer,"AT+SCAN",7))//AT+SCAN. for scan
    {
        g_scan=TRUE;
        g_scannum=0;
        HalLcdWriteString("Central Discovering",HAL_LCD_LINE_1);
        HalLcdWriteString("",HAL_LCD_LINE_2);
        GAPRole_StartDiscovery(DEVDISC_MODE_ALL,TRUE,FALSE);//Start a device discovery scan, active scan in all discovey mode not using whitelist
        //NPI_WriteTransport(&buffer[3],6);
    }
    else if (number==13 && str_cmp(buffer,"AT+CONNECT",10))//AT+CONNECT?. for connect
    {
        uint8 i;
        uint16 max,min;
        
        if (g_multi)
        {
            max=0x0000;min=0xffff;
            for (i=0;i<g_multi;i++)
            {
                max=math_max(g_central[i].interval,max);
                min=math_min(g_central[i].interval,min);
            }
            GAP_SetParamValue(TGAP_CONN_EST_INT_MIN,min);
            GAP_SetParamValue(TGAP_CONN_EST_INT_MAX,max);
        }
        GAPRole_EstablishLink(FALSE,FALSE,g_scanlist[buffer[10]-'0'].addrType,g_scanlist[buffer[10]-'0'].addr);//Establish a link to a peer device, not use high scan duty cycle and white list when creating link
        HalLcdWriteString("Central Connecting",HAL_LCD_LINE_1);
        HalLcdWriteString(address2string(g_scanlist[buffer[10]-'0'].addr),HAL_LCD_LINE_2);
        //NPI_WriteTransport(&buffer[3],10);
    }
    else if (number==11 && str_cmp(buffer,"AT+POWER",8))//AT+POWER?. for power change
    {
        HCI_EXT_SetTxPowerCmd(buffer[8]-'0');
        NPI_WriteTransport(&buffer[3],7);
    }/*
    else if (str_cmp(buffer,"AT+SEND",7))//AT+SEND???. for send data to peripheral/central
    {
        uint8 i;
        multi_t *multi;
        
        for (i=0;i<g_scannum;i++)
        {
            multi=multi_c(i);
            if (multi_connect(ROLE_CENTRAL,multi->handle_conn))
            {
                send_data(&buffer[7],number-7-2,multi,task_id);//only data except /r/n
                NPI_WriteTransport(&buffer[7],number-7-2);
            }
        }
        multi=multi_p(i);
        if (multi_connect(ROLE_PERIPHERAL,multi->handle_conn))
        {
            send_data(&buffer[7],number-7-2,multi,task_id);//only data except /r/n
            NPI_WriteTransport(&buffer[7],number-7-2);
        }
    }*/
}

void cmd_parse(uint8 *value,uint8 length,uint8 task_id)
{
    //uint8 buffer[2];
    
    if (length>2)
        return;/*
    switch(value[0])
    {
    case CMD_QUERY:
        HalLcdWriteString("Query Cmd Recv",HAL_LCD_LINE_2);
        buffer[0]=RET_QUERY;
        buffer[1]=(uint8)g_switch;
        send_data(buffer,2,&g_peripheral,task_id);
        break;
    case CMD_SWITCH:
        HalLcdWriteString("Switch Cmd Recv",HAL_LCD_LINE_2);
        if (value[1]==!g_switch)
        {
            HalLedSet(HAL_LED_1,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            HalLedSet(HAL_LED_2,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            HalLedSet(HAL_LED_3,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            g_switch=(bool)value[1];
        }
        break;
    case CMD_M2M:
        HalLcdWriteString("M2M Cmd Recv",HAL_LCD_LINE_2);
        g_m2m=TRUE;
        GAPRole_TerminateConnection(g_peripheral.handle_conn);//Terminate a link
        g_turnover=TRUE;//peripheral->central
        //peripheral auto(->start)
        //osal_set_event(task_id,APP_EVENT_INITIAL);
        break;
    case CMD_CLOCK:
        HalLcdWriteString("Switch Cmd Recv",HAL_LCD_LINE_2);
        if (value[1]==!g_switch)
        {
            HalLedSet(HAL_LED_1,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            HalLedSet(HAL_LED_2,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            HalLedSet(HAL_LED_3,value[1] ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
            g_switch=(bool)value[1];
            osal_set_event(task_id,APP_EVENT_CLOCK);
        }
        break;
    }*/
}

uint16 math_min(uint16 a,uint16 b)
{
    return a<b ? a : b;
}

uint16 math_max(uint16 a,uint16 b)
{
    return a>b ? a : b;
}

//ROLE_PERIPHERAL-peripheral,ROLE_CENTRAL-central
uint8 multi_num(uint8 role)
{
    uint8 i;
    uint8 number;

    number=0;
    switch(role)
    {
    case ROLE_CENTRAL:
        for (i=0;i<g_multi;i++)
            if (g_central[i].status==STATE_CONNECTED || g_central[i].status==STATE_SERVICES || g_central[i].status==STATE_CHARACTERISTIC)
                number++;
        break;
    case ROLE_PERIPHERAL:
        if (g_peripheral.status==STATE_CONNECTED)
            number++;
        break;
    }
    
    return number;
}

//Judge whether ble connected
//0-not connect,1-connected
bool multi_connect(uint8 role,uint16 handle)
{
    multi_t *multi;
    
    multi=role==ROLE_CENTRAL ? multi_point(handle) : &g_peripheral;
    return multi->status==STATE_CONNECTED || multi->status==STATE_SERVICES || multi->status==STATE_CHARACTERISTIC;
}

multi_t *multi_point(uint16 handle)
{
    //find index according to handle
    uint8 i;
    
    for (i=0;i<g_multi;i++)
        if (g_central[i].handle_conn==handle)
            return &g_central[i];
    
    return NULL;
}

uint8 multi_find(uint8 *addr)
{
    uint8 i;
    
    for (i=0;i<g_multi;i++)
        if (osal_memcmp(g_central[i].addr,addr,B_ADDR_LEN))
            return i;
    
    return NUM_CENTRAL;
}

uint8 multi_del(void)
{
    //del handle in multi array
    uint8 i;
    
    if (!g_multi)
        return FAILURE;
    i=0;
    while(i<g_multi)
    {
        if (g_central[i].status!=STATE_CONNECTED && g_central[i].status!=STATE_SERVICES && g_central[i].status!=STATE_CHARACTERISTIC)
        {
            g_multi--;
            for (;i<g_multi;i++)
                osal_memcpy(&g_central[i],&g_central[i+1],sizeof(multi_t));
        }
        else
            i++;
    }

    return SUCCESS;
}

bool multi_check(uint8 *addr)
{
    uint8 i;
    
    for (i=0;i<g_multi;i++)
        if (osal_memcmp(g_central[i].addr,addr,B_ADDR_LEN) && (g_central[i].status==STATE_CONNECTED || g_central[i].status==STATE_SERVICES || g_central[i].status==STATE_CHARACTERISTIC))
            return TRUE;
    
    return FALSE;
}