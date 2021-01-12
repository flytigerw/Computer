int a;
volatile int b;

void test(){
  ++b;
  a = b + 1;
}

