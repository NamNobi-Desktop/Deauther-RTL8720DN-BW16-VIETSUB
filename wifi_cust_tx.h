#ifndef WIFI_CUST_TX
#define WIFI_CUST_TX
#include <Arduino.h>


/*Từ chối trách nhiệm

Kho lưu trữ này được cung cấp "nguyên trạng" (as is), không có bất kỳ bảo đảm nào, dù rõ ràng hay ngụ ý, bao gồm nhưng không giới hạn ở các bảo đảm về tính thương mại, sự phù hợp cho một mục đích cụ thể hoặc không vi phạm quyền lợi. 
Trong mọi trường hợp, tác giả hoặc chủ sở hữu bản quyền sẽ không chịu trách nhiệm với bất kỳ yêu cầu, thiệt hại hoặc trách nhiệm nào khác, dù trong hợp đồng, lỗi dân sự hay cách khác, phát sinh từ, liên quan đến, hoặc kết nối với phần mềm hoặc việc sử dụng, khai thác phần mềm.

Việc sử dụng kho lưu trữ này và nội dung bên trong là hoàn toàn tự nguyện và chịu rủi ro từ phía người dùng. Tác giả không đảm bảo tính chính xác, đáng tin cậy hoặc đầy đủ của bất kỳ thông tin nào được cung cấp trong kho lưu trữ này.

bằng việc sử dụng kho lưu trữ này, bạn đồng ý với các điều khoản trên và xác nhận rằng bạn tự chịu trách nhiệm tuân thủ mọi quy định pháp luật hoặc quy tắc có liên quan*/






typedef struct {
  uint16_t frame_control = 0xC0;
  uint16_t duration = 0xFFFF;
  uint8_t destination[6];
  uint8_t source[6];
  uint8_t access_point[6];
  const uint16_t sequence_number = 0;
  uint16_t reason = 0x06;
} DeauthFrame;

typedef struct {
  uint16_t frame_control = 0x80;
  uint16_t duration = 0;
  uint8_t destination[6];
  uint8_t source[6];
  uint8_t access_point[6];
  const uint16_t sequence_number = 0;
  const uint64_t timestamp = 0;
  uint16_t beacon_interval = 0x64;
  uint16_t ap_capabilities = 0x21;
  const uint8_t ssid_tag = 0;
  uint8_t ssid_length = 0;
  uint8_t ssid[255];
} BeaconFrame;

extern uint8_t* rltk_wlan_info;
extern "C" void* alloc_mgtxmitframe(void* ptr);
extern "C" void update_mgntframe_attrib(void* ptr, void* frame_control);
extern "C" int dump_mgntframe(void* ptr, void* frame_control);

void wifi_tx_raw_frame(void* frame, size_t length);
void wifi_tx_deauth_frame(void* src_mac, void* dst_mac, uint16_t reason = 0x06);
void wifi_tx_beacon_frame(void* src_mac, void* dst_mac, const char *ssid);

#endif
