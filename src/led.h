#ifndef led_h
#define led_h

class LED {
  public:
    class BuiltIn {
      public:
        static void init();
        static void on();
        static void off();
        static void blink(int millis);
    };
};

#endif