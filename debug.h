#define DEBUG_MODE 0

#if DEBUG_MODE
  #define PRINT(stuff) Serial.print(stuff);
  #define PRINT_LN(stuff) Serial.println(stuff);
  #define SERIAL_BEGIN() Serial.begin(9600); while(!Serial); 
  #define SERIAL_FLUSH() Serial.flush();
#else
  #define PRINT(stuff)
  #define PRINT_LN(stuff)
  #define SERIAL_BEGIN()
  #define SERIAL_FLUSH()
#endif
