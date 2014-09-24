#include "contiki.h"
#include <string.h>

#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>

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

static const uint16_t serviceids[] = {
  0x111F, // HFP gateway
  0x110C, // AVRCP server
  0x1132, // MAP MNS Server
};
static uint8_t current_server = 0;
static bd_addr_t addr;

extern int mas_open(const bd_addr_t *remote_addr, uint8_t port);

static enum
{
  SENDING,
  RECV,
  DONE
}state;

static uint16_t l2cap_cid;
static uint16_t transitionid = 0;

static void sdpc_trysend()
{
  uint8_t buf[128];
  if (state != SENDING) return;
  if (!l2cap_cid) return;
  if (!l2cap_can_send_packet_now(l2cap_cid)) return;

  buf[0] = SDP_ServiceSearchAttributeRequest;
  net_store_16(buf, 1, transitionid++);

  uint8_t *param = &buf[5];
  de_create_sequence(param);
  de_add_number(param, DE_UUID, DE_SIZE_16, serviceids[current_server]);
  uint16_t size = de_get_len(param);
  net_store_16(param, size, 30); // max length is 30 bytes
  size+=2;
  de_create_sequence(param + size);
  //de_add_number(param + size, DE_UINT, DE_SIZE_16, SDP_ServiceClassIDList);
  de_add_number(param + size, DE_UINT, DE_SIZE_16, SDP_ProtocolDescriptorList);
  size += de_get_len(param + size);
  param[size++] = 0;

  net_store_16(buf, 3, size);
  hexdump(buf, size + 5);
  int err = l2cap_send_internal(l2cap_cid, buf, size + 5);
  if (!err)
  {
    state = RECV;
    log_info("sdp request sent.\n");
  }
  else
  {
    log_info("sdpc_trysend l2cap_send_internal error: %d\n", err);
  }
}

static uint8_t hfp_port, mas_port;
/*
parse the sdp record:
type   DES (6), element len 26
    type   DES (6), element len 24
        type  UINT (1), element len  3 , value: 0x00000004
        type   DES (6), element len 19
            type   DES (6), element len  5
                type  UUID (3), element len  3 , value: 0x00000100
            type   DES (6), element len  7
                type  UUID (3), element len  3 , value: 0x00000003
                type  UINT (1), element len  2 , value: 0x00000002
            type   DES (6), element len  5
                type  UUID (3), element len  3 , value: 0x00000008
*/
static void sdpc_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  if (packet_type == HCI_EVENT_PACKET)
  {
    switch(packet[0]){
    case L2CAP_EVENT_CHANNEL_OPENED:
      {
        if (packet[2]) {
          log_info("Connection failed\n");
          return;
        }
        log_info("Connected\n");
        state = SENDING;
        current_server = 0;
        l2cap_cid = READ_BT_16(packet, 13);
        sdpc_trysend();
        break;
      }
    case DAEMON_EVENT_HCI_PACKET_SENT:
    case L2CAP_EVENT_CREDITS:
      {
        sdpc_trysend();
        break;
      }
    case L2CAP_EVENT_CHANNEL_CLOSED:
      if (channel == l2cap_cid){
        // reset
        l2cap_cid = 0;
      }
      break;
    }
  }

  if (packet_type == L2CAP_DATA_PACKET){
    log_info("SDP Respone %d \n", READ_NET_16(packet, 5));
    //de_dump_data_element(&packet[7]);

    //check if valid answer returns
    if (READ_NET_16(packet, 5) > 2)
    {
      switch(current_server)
      {
      case 0:
        {
        hfp_port = sdp_get_parameters_for_uuid(&packet[7], 0x0003);;
        log_info("hfp port: %d\n", hfp_port);
        break;
        }
      case 1:
        break;
      case 2:
        {
        mas_port = sdp_get_parameters_for_uuid(&packet[7], 0x0003);;
        log_info("MAP port: %d\n", mas_port );
        break;
        }
      }
    }

    current_server++;
    if (current_server == 3)
    {
      state = DONE;
      l2cap_close_connection(&current_server);
      if (hfp_port != 0)
        hfp_open(&addr, hfp_port);
//      if (mas_port != 0)
//        mas_open(&addr, mas_port);
    }
    else
    {
      state = SENDING;
      sdpc_trysend();
    }
  }
}


void sdpc_open(bd_addr_t remote_addr)
{
  if (l2cap_cid != 0)
    return;

  if (*(uint32_t*)remote_addr == 0)
    return;

  memcpy(addr, remote_addr, sizeof(addr));
  l2cap_create_channel_internal(&current_server, sdpc_packet_handler, remote_addr , PSM_SDP, 0xffff);
}