#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

//#define DEBUG
#define DEBUG_BAUD 115200

#ifdef DEBUG
  #define DEBUG_SER_INIT() Serial.begin(DEBUG_BAUD);
  #define DEBUG_SER_PRINT(...) Serial.print(__VA_ARGS__);
#else
  #define DEBUG_SER_PRINT(...)
  #define DEBUG_SER_INIT()
#endif

#endif
//Từ chối trách nhiệm
//Việc sử dụng kho lưu trữ này và nội dung bên trong là hoàn toàn tự nguyện và chịu rủi ro từ phía người dùng. Tác giả không đảm bảo tính chính xác, đáng tin cậy hoặc đầy đủ của bất kỳ thông tin nào được cung cấp trong kho lưu trữ này.
//Bằng việc sử dụng kho lưu trữ này, bạn đồng ý với các điều khoản trên và xác nhận rằng bạn tự chịu trách nhiệm tuân thủ mọi quy định pháp luật hoặc quy tắc có liên quan.