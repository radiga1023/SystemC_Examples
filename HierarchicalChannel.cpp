#include "systemc.h"
class slave_if : virtual public sc_interface {
  public:
  	virtual void read(unsigned int address, unsigned int& data) = 0 ; 
    virtual void write(unsigned int address, unsigned int data) = 0 ;
};
class peripheral_if : public sc_interface {
    public:
  	virtual unsigned int read(unsigned int address) = 0 ; 
    virtual void write(unsigned int address, unsigned int data) = 0 ;
};
class peripheral : public sc_channel, public peripheral_if {
 protected:
  	unsigned int pdata_val[250];
  public:
  unsigned int read(unsigned int address) {
    if(address < 250) {
      std::cout << "Reading..." << pdata_val[address]<< std::endl;
      return pdata_val[address] ;
  }
  }
  void write(unsigned int address, unsigned int data) {
    if(address < 250) {
      pdata_val[address] = data ;
      std::cout << "Writing..." << pdata_val[address]<< std::endl;
    }
  }
  SC_HAS_PROCESS(peripheral);
  peripheral(sc_module_name name) : sc_channel(name) { }
};
class slave : public sc_channel, public slave_if {
  protected:
  	unsigned int data_val[250];
  public:
  sc_port<peripheral_if> in ;
  void read(unsigned int address, unsigned int& data) {
    if(address < 250) {
      in->read(address);
    }
  }
  void write(unsigned int address, unsigned int data) {
    if(address < 250) {
      in->write(address, data);
    }
  }
  SC_HAS_PROCESS(slave);
  slave(sc_module_name name) : sc_module(name) { }
};
class master : public sc_module {
  public:
  sc_port<slave_if> out ;
  SC_HAS_PROCESS(master);
  master(sc_module_name name):sc_module(name) {
    SC_THREAD(master_xfers);
  }
  void master_xfers() {
    unsigned int w_data, r_data;
  	//Write Data to Channel
    for(int i = 0; i < 10; i++) {
      wait(10, SC_NS) ;
      w_data = i * 3 ;
      out->write(i, w_data);
    }
    //Read data From Channel
    for(int i = 0; i < 10; i++) {
      wait(10, SC_NS);
      out->read(i, r_data);
     // std::cout << "Data Read@ Slave If " << r_data << std::endl ;
    }
  }
};

class top : public sc_module {
  public:
  	master master_inst ;
    slave slave_inst ;
    peripheral peri_inst;
    SC_HAS_PROCESS(top);
  top(sc_module_name name) : sc_module(name), master_inst("MasterMod"), slave_inst("SlaveMod"), peri_inst("Peripheral"){
    master_inst.out(slave_inst);
    slave_inst.in(peri_inst);
  }
};
int sc_main(int argc, char** argv) {
  top top1 ("TopModule");
  sc_start();
  return 0;
}
