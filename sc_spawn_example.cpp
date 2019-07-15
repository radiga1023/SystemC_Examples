// Code your testbench here.
// Uncomment the next line for SystemC modules.
#include "systemc.h"
#include <iostream>

using namespace std;

class MyInterface : virtual public sc_interface
{
  public: 
  virtual void write(unsigned int address, unsigned char* data) = 0;
  virtual void read(unsigned int address, unsigned char* data) = 0;
};

class Producer : public sc_module
{
  public:
  sc_port<MyInterface> in_port;
  SC_HAS_PROCESS(Producer);
  Producer(sc_module_name nm_) : sc_module(nm_)
  {
    //in_port.bind(*this);
    SC_THREAD(generator);
  }
    void spawned_thread(bool value)
  {
    cout << "Spawned thread created successfully with value " << value << endl;
    unsigned int count = 0;
    unsigned char* data;
    data = new unsigned char[100];
    for(unsigned int i = 0; i <=50; i++) data[i] = (value == 1)?0x66:0x99;
    cout << "Initiating WRITE calls from spawned thread" << endl;
    while(count < 10)
    {
      in_port->write(count, data);
      wait(1, SC_NS);
      count+=1;
    }
  }
  void generator()
  {
    sc_spawn(sc_bind(&Producer::spawned_thread, this, false));
    sc_spawn(sc_bind(&Producer::spawned_thread, this, true));
    unsigned int count = 0;
    unsigned char* data;
    data = new unsigned char[100];
    for(unsigned int i = 0; i <=50; i++) data[i] = 0x55;
    cout << "Initiating WRITE calls from generator" << endl;
    while(count < 10)
    {
      in_port->write(count, data);
      wait(1, SC_NS);
      count+=1;
    }
  }
  

};

class Consumer : public sc_module, public MyInterface
{
  public:
  sc_export<MyInterface> in_port;
  
  unsigned char* MEM = new unsigned char[100];
  
  SC_HAS_PROCESS(Consumer);
  
  
  Consumer(sc_module_name nm_) : sc_module(nm_)
  {
    in_port.bind(*this);
  }
  void write(unsigned int address, unsigned char* data)
  {
    cout << "Data Write Call Arrived at Consumer " << endl;
    memcpy(MEM, data, 10);
    for (unsigned int i = 0; i < 10; i++)
    {
      cout << hex << (unsigned int)MEM[i] << " " ;
    }
    cout << endl << endl;
  }
  void read(unsigned int address, unsigned char* data)
  {
    memcpy(data, MEM, 10);
  }
};

class top : public sc_module
{
  public:
  top(sc_module_name nm_) : sc_module(nm_)
  {
    prod = new Producer("Producer");
    cons = new Consumer("Consumer");
    prod->in_port.bind(cons->in_port);
  }
  private:
  Producer* prod;
  Consumer* cons;
};

int sc_main(int argc, char** argv)
{
  top tp("top");
  sc_start();
  return 0;
}
