//Run the below code in EDA playground
//https://www.edaplayground.com/x/2rer

#include "systemc.h"
class producer : public sc_module {
  public:
  sc_port<sc_fifo_out_if<char> > out ;
  SC_HAS_PROCESS(producer);
  producer(sc_module_name name) : sc_module(name) {
    SC_THREAD(main);
  }
  void main() {
    const char* str = "Producer and Consumer Model" ;
    std::cout << "Writing to Channel" << std::endl;
    while(*str) {
      std::cout << *str;
      out->write(*str);
      *str++ ;
     // wait(2, SC_NS);
    }
  }
  };
class consumer : public sc_module {
      public:
      sc_export<sc_fifo_in_if<char> > in ;
      SC_HAS_PROCESS(consumer);
      consumer(sc_module_name name) : sc_module(name) {
        SC_THREAD(main);
      }
      void main() {
        char c ;
        std::cout << std::endl;
        std::cout << "Reading at Consumer Side" << std::endl;
        while(true) {
          in->read(c);
          std::cout << c << flush;
        }
      }
};
class top : public sc_module {
          public:
          	producer prod_inst;
            consumer cons_inst;
          sc_fifo<char> fifo_inst ;
          SC_HAS_PROCESS(top);
          top(sc_module_name name) : sc_module(name),fifo_inst("FIFO"), prod_inst("Producer"),cons_inst("Consumer") {
            prod_inst.out(fifo_inst);
            cons_inst.in(fifo_inst);
          }
};
int sc_main(int argc, char** argv) {
            top t1("TopModule");
            sc_start();
            return 0;
}

      	
