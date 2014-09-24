
#include "contiki.h"
#include "window.h"

#include <string.h>

#include "bt_control_cc256x.h"

#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>

#include "btstack-config.h"
#include "hci.h"
#include "l2cap.h"
#include "btstack_memory.h"
#include "remote_device_db.h"
#include "rfcomm.h"
#include "sdp.h"
#include "hfp.h"
#include "avctp.h"
#include "avrcp.h"

#include "debug.h"

#include "bluetooth.h"

#include "system.h"

extern void ble_init(void);
extern void ble_start(void);
extern void deviceid_init(void);
extern void spp_init(void);
extern void sdpc_open(const bd_addr_t remote_addr);
extern void ant_shutdown(void);
extern void ble_advertise(uint8_t onoff);

//static bd_addr_t currentbd;
static uint8_t running = 0;
#if PRODUCT_W001
static uint16_t handle_audio = 0;
#endif

// Bluetooth logic
static void packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{

  // handle events, ignore data
  if (packet_type != HCI_EVENT_PACKET)
  {
    return;
  }

  switch (packet[0]) {
  case BTSTACK_EVENT_STATE:
    // bt stack activated, get started - set local name
    if (packet[2] == HCI_STATE_WORKING) {
      log_info("Start initialize bluetooth chip!\n");
      running = 1;
      hci_send_cmd(&hci_vs_write_sco_config, 0x00, 120, 720, 0x01);
      process_post(ui_process, EVENT_BT_STATUS, (void*)BT_INITIALIZED);
    }
    else if (packet[2] == HCI_STATE_OFF)
    {
      running = 0;
      // close the connection
      process_exit(&bluetooth_process);
      
      bluetooth_platform_shutdown();

      // notify UI that we are shutdown
      process_post(ui_process, EVENT_BT_STATUS, (void*)BT_SHUTDOWN);
    }
    break;

  case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED:
    {
      if (packet[2]) {
        process_post(ui_process, EVENT_BT_STATUS, (void*)BT_CONNECTED);
      } else {
        process_post(ui_process, EVENT_BT_STATUS, (void*)BT_DISCONNECTED);
      }
      break;
    }
  case HCI_EVENT_CONNECTION_REQUEST:
    {
      log_info("connection request\n");
      break;
    }
#ifdef PRODUCT_W001
  case HCI_EVENT_CONNECTION_COMPLETE:
  {
    if (packet[11] == 2)
    {
      handle_audio = READ_BT_16(packet, 3);
      codec_wakeup();
    }
    break;
  }
  case HCI_EVENT_DISCONNECTION_COMPLETE:
    {
      uint16_t handle = READ_BT_16(packet, 3);
      if (handle == handle_audio)
      {
        handle_audio = 0;
        codec_suspend();
      }
      break;
    }
#endif
  case HCI_EVENT_LINK_KEY_NOTIFICATION:
    {
      //bd_addr_t event_addr;
      // new device is paired
      //bt_flip_addr(event_addr, &packet[2]);
      //memcpy(&currentbd, event_addr, sizeof(currentbd));
      //sdpc_open(event_addr);
      // close bluetooth information page
      break;
    }
    case HCI_EVENT_COMMAND_COMPLETE:
    {
      if (COMMAND_COMPLETE_EVENT(packet, hci_vs_write_sco_config)){
        hci_send_cmd(&hci_vs_write_codec_config, 2048, 0, (uint32_t)8000, 0, 1, 0, 0,
                       16, 1, 0, 16, 1, 1, 0,
                       16, 40, 0, 16, 40, 1, 0
                       );
        break;
      }
      else if (COMMAND_COMPLETE_EVENT(packet, hci_vs_write_codec_config)){
        hci_send_cmd(&hci_write_default_link_policy_settings, 0x000F);
        break;
      }
      else if (COMMAND_COMPLETE_EVENT(packet, hci_write_default_link_policy_settings)) {
        process_post(ui_process, EVENT_BT_STATUS, (void*)BT_INITIALIZED);
        printf("\n$$OK BLUETOOTH\n");

        // as testing
        if (system_testing())
        {
#if PRODUCT_W001
          ant_shutdown();
#endif
          printf("\n$$END\n");
        }
      }
      break;
    }
  }
}

static void btstack_setup(){
  /// GET STARTED with BTstack ///
  btstack_memory_init();

  // init HCI
  hci_transport_t    * transport = hci_transport_h4_dma_instance();
  bt_control_t       * control   = bt_control_cc256x_instance();
  hci_uart_config_t  * config    = hci_uart_config_cc256x_instance();
  remote_device_db_t * remote_db = (remote_device_db_t *) &remote_device_db_memory;
  hci_init(transport, config, control, remote_db);

  // use eHCILL
  bt_control_cc256x_enable_ehcill(1);

  // Secure Simple Pairing configuration -> just works
  hci_ssp_set_enable(1);
  hci_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  hci_ssp_set_auto_accept(1);

  hci_set_class_of_device(0x7C0704);

  // lower the init power
  // bt_control_cc256x_set_power(-40);

  // init L2CAP
  l2cap_init();
  l2cap_register_packet_handler(packet_handler);

  // init RFCOMM
  rfcomm_init();
  rfcomm_set_required_security_level(LEVEL_0);

  // set up BLE
  ble_init();

  // init SDP, create record for SPP and register with SDP
  sdp_init();

  // register device id record
  //deviceid_init();

  spp_init();

  avctp_init();
  avrcp_init();
  
#ifdef PRODUCT_W001
  hfp_init();
#endif
  //mns_init();
  bluetooth_platform_init();
}

void bluetooth_init()
{
  btstack_setup();
#ifdef PRODUCT_W001
  codec_init(); // init codec
#endif
  process_start(&bluetooth_process, NULL);
}

void bluetooth_shutdown()
{
#ifdef PRODUCT_W001
  codec_shutdown(); // init codec
#endif

  hci_power_control(HCI_POWER_OFF);

  running = 0;
}

void bluetooth_start()
{
  ble_start();
  bluetooth_platform_init();

  process_start(&bluetooth_process, NULL);
}

void bluetooth_discoverable(uint8_t onoff)
{
  hci_discoverable_control(onoff);
  ble_advertise(onoff);
}

uint8_t bluetooth_running()
{
  return running;
}

const char* bluetooth_address()
{
  return (const char*)hci_local_bd_addr();
}

static void dut_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  // handle events, ignore data
  if (packet_type != HCI_EVENT_PACKET)
  {
    return;
  }

  switch (packet[0]) {
  case HCI_EVENT_COMMAND_COMPLETE:
    {
      if (COMMAND_COMPLETE_EVENT(packet, hci_set_event_filter)){
        log_info("set event filter done. ret=%x\n", packet[OFFSET_OF_DATA_IN_COMMAND_COMPLETE]);
        hci_send_cmd(&hci_write_scan_enable, 0x03);
        break;
      }
      else if (COMMAND_COMPLETE_EVENT(packet, hci_write_scan_enable)){
        log_info("scan enable finish. ret=%x \n", packet[OFFSET_OF_DATA_IN_COMMAND_COMPLETE]);
        hci_send_cmd(&hci_enable_device_under_test_mode);
        break;
      }
      else if (COMMAND_COMPLETE_EVENT(packet, hci_enable_device_under_test_mode)){
        log_info("device is in DUT mode. ret = %x\n", packet[OFFSET_OF_DATA_IN_COMMAND_COMPLETE]);
      }
    }
  }
}

void bluetooth_enableDUTMode()
{
  // enable DUT
  l2cap_register_packet_handler(dut_packet_handler);

  hci_send_cmd(&hci_set_event_filter, 0x02, 0x00, 0x03);
}


#define COMMAND_COMPLETE_EVENT_RAW(event, opcode) ( event[0] == HCI_EVENT_COMMAND_COMPLETE && READ_BT_16(event,3) == opcode)
static const uint8_t HCI_VS_DRPb_Tester_Con_TX_Cmd[] = 
{
  0x84, 0xFD, 12, 0x00, 0x00, 0x00, 0x07, 0, 0, 0, 0, 0, 0, 0, 0
};
static const uint8_t HCI_VS_Write_Hardware_Register_Cmd[] = 
{
  0x01, 0xFF, 6, 0x0c, 0x18, 0x19, 0x00, 0x01, 0x01
};
static const uint8_t HCI_VS_DRPb_Enable_RF_Calibration_Cmd[] =
{
  0x80, 0xFD, 6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01
};

static void contx_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  // handle events, ignore data
  if (packet_type != HCI_EVENT_PACKET)
  {
    return;
  }

  switch (packet[0]) {
  case HCI_EVENT_COMMAND_COMPLETE:
    {
      if (COMMAND_COMPLETE_EVENT_RAW(packet, 0xFD84)){
        log_info("HCI_VS_DRPb_Tester_Con_TX_Cmd done.\n");
        hci_send_cmd_packet((uint8_t*)HCI_VS_Write_Hardware_Register_Cmd, sizeof(HCI_VS_Write_Hardware_Register_Cmd));
        break;
      }
      else if (COMMAND_COMPLETE_EVENT_RAW(packet, 0xFF01)){
        log_info("HCI_VS_Write_Hardware_Register_Cmd done.\n");
        hci_send_cmd_packet((uint8_t*)HCI_VS_DRPb_Enable_RF_Calibration_Cmd, sizeof(HCI_VS_DRPb_Enable_RF_Calibration_Cmd));
        break;
      }
      else if (COMMAND_COMPLETE_EVENT_RAW(packet, 0xFD80)){
        log_info("HCI_VS_DRPb_Enable_RF_Calibration_Cmd done.\n");
        break;
      }
    }
  }
}

void bluetooth_enableConTxMode(int mode, int freq)
{

  l2cap_register_packet_handler(contx_packet_handler);
  uint8_t buf[sizeof(HCI_VS_DRPb_Tester_Con_TX_Cmd)];
    memcpy(buf, HCI_VS_DRPb_Tester_Con_TX_Cmd, sizeof(HCI_VS_DRPb_Tester_Con_TX_Cmd));
    buf[3] = mode;
    buf[5] = freq;
    hci_send_cmd_packet(buf, sizeof(HCI_VS_DRPb_Tester_Con_TX_Cmd));
}
