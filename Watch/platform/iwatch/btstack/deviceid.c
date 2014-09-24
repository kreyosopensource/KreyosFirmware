#include "contiki.h"

#include "hci.h"
#include "sdp.h"
#include "btstack/sdp_util.h"

#include <string.h>
#include "btstack-config.h"
#include "debug.h"


static service_record_item_t did_service_record;
static uint8_t   did_service_buffer[100];


static void sdp_create_did_service(uint8_t *service){

  uint8_t* attribute;
  de_create_sequence(service);

  // 0x0000 "Service Record Handle"
  de_add_number(service, DE_UINT, DE_SIZE_16, SDP_ServiceRecordHandle);
  de_add_number(service, DE_UINT, DE_SIZE_32, 0x10009);

  de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_ServiceClassIDList);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UUID, DE_SIZE_16, 0x1200 );
  }
  de_pop_sequence(service, attribute);

  de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_SpecificationID);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0x0103 );
  }
  de_pop_sequence(service, attribute);

   de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_VendorID);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0xFFFF );
  }
  de_pop_sequence(service, attribute);

   de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_ProductID);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0x0001 );
  }
  de_pop_sequence(service, attribute);

   de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_Version);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0x0001);
  }
  de_pop_sequence(service, attribute);

   de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_PrimaryRecord);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_BOOL, DE_SIZE_8, 1);
  }
  de_pop_sequence(service, attribute);

   de_add_number(service,  DE_UINT, DE_SIZE_16, SDP_VendorIDSource);
  attribute = de_push_sequence(service);
  {
    de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0x0001);
  }
  de_pop_sequence(service, attribute);
}

void deviceid_init()
{
  memset(&did_service_record, 0, sizeof(did_service_record));
  did_service_record.service_record = (uint8_t*)&did_service_buffer[0];
#if 1
  sdp_create_did_service((uint8_t*)did_service_record.service_record);
  log_info("SDP service buffer size: %u\n", de_get_len(did_service_record.service_record));
  //hexdump((void*)did_service_buffer, de_get_len(did_service_record.service_record));
  //de_dump_data_element(did_service_record.service_record);
#endif
  sdp_register_service_internal(NULL, &did_service_record);
}