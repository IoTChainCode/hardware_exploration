/******************************************************************************

 @file  central.c

 @brief GAP Central Role

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2011-2016, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: ble_sdk_1.4.2.2
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "osal_cbtimer.h"
#include "osal_snv.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gap.h"
#include "gapbondmgr.h"
#include "central.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Profile Events
#define START_ADVERTISING_EVT         0x0001
#define RSSI_READ_EVT                 0x0002
#define UPDATE_PARAMS_TIMEOUT_EVT     0x0004

// Profile OSAL Message IDs
#define GAPRole_RSSI_MSG_EVT   0xE0

/*********************************************************************
 * TYPEDEFS
 */

// RSSI read data structure
typedef struct
{
  uint16        period;
  uint16        connHandle;
  uint8         timerId;
} gapRoleRssi_t;

// OSAL event structure for RSSI timer events
typedef struct
{
  osal_event_hdr_t      hdr;
  gapRoleRssi_t  *pRssi;
} gapRoleRssiEvent_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Task ID
static uint8 gapRoleTaskId;

// App callbacks
static gapRoleCB_t *pgapRoleCB;

// Array of RSSI read structures
static gapRoleRssi_t gapRoleRssi[GAPRole_NUM_RSSI_LINKS];

/*********************************************************************
 * Profile Parameters - reference GAPRole_PROFILE_PARAMETERS for
 * descriptions
 */

static uint8  gapRoleIRK[KEYLEN];
static uint8  gapRoleSRK[KEYLEN];
static uint32 gapRoleSignCounter;
static uint8  gapRoleBdAddr[B_ADDR_LEN];
static uint8  gapRoleMaxScanRes = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 GAPRole_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void GAPRole_ProcessGattEvent(gattMsgEvent_t *pMsg);
static uint8 GAPRole_ProcessGAPMsg( gapEventHdr_t *pMsg );
static gapRoleRssi_t *GAPRole_RssiAlloc( uint16 connHandle );
static gapRoleRssi_t *GAPRole_RssiFind( uint16 connHandle );
static void GAPRole_RssiFree( uint16 connHandle );
static void GAPRole_timerCB( uint8 *pData );

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Start the device in Central role.  This function is typically
 *          called once during system startup.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_StartDevice( gapRoleCB_t *pAppCallbacks )
{
  if ( pAppCallbacks )
  {
    pgapRoleCB = pAppCallbacks;
  }

  return GAP_DeviceInit( gapRoleTaskId, GAP_PROFILE_CENTRAL,
                         gapRoleMaxScanRes, gapRoleIRK,
                         gapRoleSRK, &gapRoleSignCounter );
}

/**
 * @brief   Set a parameter in the Central Profile.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_SetParameter( uint16 param, uint8 len, void *pValue )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case GAPRole_IRK:
      if ( len == KEYLEN )
      {
        VOID osal_memcpy( gapRoleIRK, pValue, KEYLEN ) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPRole_SRK:
      if ( len == KEYLEN )
      {
        VOID osal_memcpy( gapRoleSRK, pValue, KEYLEN ) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPRole_SIGNCOUNTER:
      if ( len == sizeof ( uint32 ) )
      {
        gapRoleSignCounter = *((uint32*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPRole_MAX_SCAN_RES:
      if ( len == sizeof ( uint8 ) )
      {
        gapRoleMaxScanRes = *((uint8*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ret;
}

/**
 * @brief   Get a parameter in the Central Profile.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_GetParameter( uint16 param, void *pValue )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case GAPRole_IRK:
      VOID osal_memcpy( pValue, gapRoleIRK, KEYLEN ) ;
      break;

    case GAPRole_SRK:
      VOID osal_memcpy( pValue, gapRoleSRK, KEYLEN ) ;
      break;

    case GAPRole_SIGNCOUNTER:
      *((uint32*)pValue) = gapRoleSignCounter;
      break;

    case GAPRole_BD_ADDR:
      VOID osal_memcpy( pValue, gapRoleBdAddr, B_ADDR_LEN ) ;
      break;

    case GAPRole_MAX_SCAN_RES:
      *((uint8*)pValue) = gapRoleMaxScanRes;
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ret;
}

/**
 * @brief   Terminate a link.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_TerminateLink( uint16 connHandle )
{
    return GAP_TerminateLinkReq( gapRoleTaskId, connHandle, HCI_DISCONNECT_REMOTE_USER_TERM ) ;
}

/**
 * @brief   Establish a link to a peer device.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_EstablishLink( uint8 highDutyCycle, uint8 whiteList,
                                        uint8 addrTypePeer, uint8 *peerAddr )
{
  gapEstLinkReq_t params;

  params.taskID = gapRoleTaskId;
  params.highDutyCycle = highDutyCycle;
  params.whiteList = whiteList;
  params.addrTypePeer = addrTypePeer;
  VOID osal_memcpy( params.peerAddr, peerAddr, B_ADDR_LEN );

  return GAP_EstablishLinkReq( &params );
}

/**
 * @brief   Update the link connection parameters.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_UpdateLink( uint16 connHandle, uint16 connIntervalMin,
                                     uint16 connIntervalMax, uint16 connLatency,
                                     uint16 connTimeout )
{
  return (bStatus_t) HCI_LE_ConnUpdateCmd( connHandle, connIntervalMin,
                                            connIntervalMax, connLatency,
                                            connTimeout, 0, 0 );
}

/**
 * @brief   Start a device discovery scan.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_StartDiscovery( uint8 mode, uint8 activeScan, uint8 whiteList )
{
  gapDevDiscReq_t params;

  params.taskID = gapRoleTaskId;
  params.mode = mode;
  params.activeScan = activeScan;
  params.whiteList = whiteList;

  return GAP_DeviceDiscoveryRequest( &params );
}

/**
 * @brief   Cancel a device discovery scan.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_CancelDiscovery( void )
{
  return GAP_DeviceDiscoveryCancel( gapRoleTaskId );
}

/**
 * @brief   Start periodic RSSI reads on a link.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_StartRssi( uint16 connHandle, uint16 period )
{
  gapRoleRssi_t  *pRssi;

  // Verify link is up
  if (!linkDB_Up(connHandle))
  {
    return bleIncorrectMode;
  }

  // If already allocated
  if ((pRssi = GAPRole_RssiFind( connHandle )) != NULL)
  {
    // Stop timer
    osal_CbTimerStop( pRssi->timerId );
  }
  // Allocate structure
  else if ((pRssi = GAPRole_RssiAlloc( connHandle )) != NULL)
  {
    pRssi->period = period;
  }
  // Allocate failed
  else
  {
    return bleNoResources;
  }

  // Start timer
  osal_CbTimerStart( GAPRole_timerCB, (uint8 *) pRssi,
                     period, &pRssi->timerId );

  return SUCCESS;
}

/**
 * @brief   Cancel periodic RSSI reads on a link.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPRole_CancelRssi(uint16 connHandle )
{
  gapRoleRssi_t  *pRssi;

  if ((pRssi = GAPRole_RssiFind( connHandle )) != NULL)
  {
    // Stop timer
    osal_CbTimerStop( pRssi->timerId );

    // Free RSSI structure
    GAPRole_RssiFree( connHandle );

    return SUCCESS;
  }

  // Not found
  return bleIncorrectMode;
}

/**
 * @brief   Central Profile Task initialization function.
 *
 * @param   taskId - Task ID.
 *
 * @return  void
 */
void GAPRole_Init( uint8 taskId )
{
  uint8 i;

  gapRoleTaskId = taskId;

  // Initialize internal data
  for ( i = 0; i < GAPRole_NUM_RSSI_LINKS; i++ )
  {
    gapRoleRssi[i].connHandle = GAP_CONNHANDLE_ALL;
    gapRoleRssi[i].timerId = INVALID_TIMER_ID;
  }

  // Initialize parameters

  // Retore items from NV
  VOID osal_snv_read( BLE_NVID_IRK, KEYLEN, gapRoleIRK );
  VOID osal_snv_read( BLE_NVID_CSRK, KEYLEN, gapRoleSRK );
  VOID osal_snv_read( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapRoleSignCounter );

  // Register for HCI/Host messages (for RSSI)
  GAP_RegisterForMsgs( taskId );
}

/**
 * @brief   Central Profile Task event processing function.
 *
 * @param   taskId - Task ID
 * @param   events - Events.
 *
 * @return  events not processed
 */
uint16 GAPRole_ProcessEvent( uint8 taskId, uint16 events)
{
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( gapRoleTaskId )) != NULL )
    {
      if ( GAPRole_ProcessOSALMsg( (osal_event_hdr_t *) pMsg ) )
      {
        // Release the OSAL message
        VOID osal_msg_deallocate( pMsg );
      }
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & GAP_EVENT_SIGN_COUNTER_CHANGED )
  {
    // Sign counter changed, save it to NV
    VOID osal_snv_write( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapRoleSignCounter );

    return ( events ^ GAP_EVENT_SIGN_COUNTER_CHANGED );
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      GAPRole_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming GAP message, FALSE otherwise.
 */
static uint8 GAPRole_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  uint8 safeToDealloc = TRUE;
  
  switch ( pMsg->event )
  {
    case HCI_GAP_EVENT_EVENT:
      if ( pMsg->status == HCI_COMMAND_COMPLETE_EVENT_CODE )
      {
        hciEvt_CmdComplete_t *pPkt = (hciEvt_CmdComplete_t *) pMsg;

        if ( pPkt->cmdOpcode == HCI_READ_RSSI )
        {
          uint16 connHandle = BUILD_UINT16( pPkt->pReturnParam[1], pPkt->pReturnParam[2] );
          int8 rssi = (int8) pPkt->pReturnParam[3];

          // Report RSSI to app
          if ( pgapRoleCB && pgapRoleCB->rssiCB )
          {
            pgapRoleCB->rssiCB( connHandle, rssi );
          }
        }
      }
      break;

    case GATT_MSG_EVENT:
      // Process GATT event
      GAPRole_ProcessGattEvent((gattMsgEvent_t *)pMsg);
      break;
      
    case GAP_MSG_EVENT:
      safeToDealloc = GAPRole_ProcessGAPMsg( (gapEventHdr_t *) pMsg );
      break;

    case GAPRole_RSSI_MSG_EVT:
      {
        gapRoleRssi_t *pRssi = ((gapRoleRssiEvent_t *) pMsg)->pRssi;

        // If link is up and RSSI reads active
        if (pRssi->connHandle != GAP_CONNHANDLE_ALL &&
            linkDB_Up(pRssi->connHandle))
        {
          // Restart timer
          osal_CbTimerStart( GAPRole_timerCB, (uint8 *) pRssi,
                             pRssi->period, &pRssi->timerId );

          // Read RSSI
          VOID HCI_ReadRssiCmd( pRssi->connHandle );
        }
      }
      break;

    default:
      break;
  }
  
  return ( safeToDealloc );
}

/*********************************************************************
 * @fn      GAPRole_ProcessGattEvent
 *
 * @brief   Process an incoming GATT Event.
 *
 * @return  None.
 */
static void GAPRole_ProcessGattEvent(gattMsgEvent_t *pMsg)
{
  switch (pMsg->method)
  {
    case ATT_FLOW_CTRL_VIOLATED_EVENT:
      // ATT request-response or indication-confirmation flow control is
      // violated. All subsequent ATT requests or indications will be dropped.
      // The profile is informed in case it wants to drop the connection.
      break;
      
    case ATT_MTU_UPDATED_EVENT:
      // MTU size updated
      break;
      
    default:
      break;
  }
}

/*********************************************************************
 * @fn      GAPRole_ProcessGAPMsg
 *
 * @brief   Process an incoming task message from GAP.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming GAP message, FALSE otherwise.
 */
static uint8 GAPRole_ProcessGAPMsg( gapEventHdr_t *pMsg )
{
  switch ( pMsg->opcode )
  {
    case GAP_DEVICE_INIT_DONE_EVENT:
      {
        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *) pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
          // Save off the generated keys
          VOID osal_snv_write( BLE_NVID_IRK, KEYLEN, gapRoleIRK );
          VOID osal_snv_write( BLE_NVID_CSRK, KEYLEN, gapRoleSRK );

          // Save off the information
          VOID osal_memcpy( gapRoleBdAddr, pPkt->devAddr, B_ADDR_LEN );
        }
      }
      break;

    case GAP_LINK_ESTABLISHED_EVENT:
      {
        gapEstLinkReqEvent_t *pPkt = (gapEstLinkReqEvent_t *) pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
          // Notify the Bond Manager of the connection
          VOID GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr,
                                   pPkt->connectionHandle, GAP_PROFILE_CENTRAL );
        }
      }
      break;

    case GAP_LINK_TERMINATED_EVENT:
      {
        uint16 connHandle = ((gapTerminateLinkEvent_t *) pMsg)->connectionHandle;

        VOID GAPBondMgr_ProcessGAPMsg( (gapEventHdr_t *)pMsg );

        // Cancel RSSI reads
        GAPRole_CancelRssi( connHandle );
      }
      break;

    // temporary workaround
    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
      VOID GAPBondMgr_ProcessGAPMsg( pMsg );
      break;

    default:
      break;
  }

  // Pass event to app
  if ( pgapRoleCB && pgapRoleCB->eventCB )
  {
    return ( pgapRoleCB->eventCB( (gapRoleEvent_t *)pMsg ) );
  }
  
  return ( TRUE );
}

/*********************************************************************
 * @fn      GAPRole_RssiAlloc
 *
 * @brief   Allocate an RSSI structure.
 *
 * @param   connHandle - Connection handle
 *
 * @return  pointer to structure or NULL if allocation failed.
 */
static gapRoleRssi_t *GAPRole_RssiAlloc( uint16 connHandle )
{
  uint8 i;

  // Find free RSSI structure
  for ( i = 0; i < GAPRole_NUM_RSSI_LINKS; i++ )
  {
    if ( gapRoleRssi[i].connHandle == GAP_CONNHANDLE_ALL )
    {
      gapRoleRssi[i].connHandle = connHandle;
      return &gapRoleRssi[i];
    }
  }

  // No free structure found
  return NULL;
}

/*********************************************************************
 * @fn      GAPRole_RssiFind
 *
 * @brief   Find an RSSI structure.
 *
 * @param   connHandle - Connection handle
 *
 * @return  pointer to structure or NULL if not found.
 */
static gapRoleRssi_t *GAPRole_RssiFind( uint16 connHandle )
{
  uint8 i;

  // Find free RSSI structure
  for ( i = 0; i < GAPRole_NUM_RSSI_LINKS; i++ )
  {
    if ( gapRoleRssi[i].connHandle == connHandle )
    {
      return &gapRoleRssi[i];
    }
  }

  // Not found
  return NULL;
}

/*********************************************************************
 * @fn      GAPRole_RssiFree
 *
 * @brief   Free an RSSI structure.
 *
 * @param   connHandle - Connection handle
 *
 * @return  none
 */
static void GAPRole_RssiFree( uint16 connHandle )
{
  uint8 i;

  // Find RSSI structure
  for ( i = 0; i < GAPRole_NUM_RSSI_LINKS; i++ )
  {
    if ( gapRoleRssi[i].connHandle == connHandle )
    {
      gapRoleRssi[i].connHandle = GAP_CONNHANDLE_ALL;
      break;
    }
  }
}

/*********************************************************************
 * @fn      GAPRole_timerCB
 *
 * @brief   OSAL timer callback function
 *
 * @param   pData - Data pointer
 *
 * @return  none
 */
static void GAPRole_timerCB( uint8 *pData )
{
  gapRoleRssiEvent_t *pMsg;

  // Timer has expired so clear timer ID
  ((gapRoleRssi_t *) pData)->timerId = INVALID_TIMER_ID;

  // Send OSAL message
  pMsg = (gapRoleRssiEvent_t *) osal_msg_allocate( sizeof(gapRoleRssiEvent_t) );
  if ( pMsg )
  {
    pMsg->hdr.event = GAPRole_RSSI_MSG_EVT;
    pMsg->pRssi = (gapRoleRssi_t *) pData;

    osal_msg_send ( gapRoleTaskId, (uint8 *) pMsg );
  }
}

/*********************************************************************
*********************************************************************/
