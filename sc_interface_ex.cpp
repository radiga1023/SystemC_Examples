#include <systemc>

class myInterface : public sc_interface
{
  public:
    virtual void read(unsigned int address, unsigned int& data) = 0;
    virtual void write(unsigned int address, unsigned int data) = 0 ;
};

class channel : public myInterface
{
  private:
    unsigned int data_val[250];
  public:
    void read(unsigned int address, unsigned int& data)
    {
      if (address < 250)
      {
        data = data_val[address];
        cout << "Reading data " << hex << data << " @ address: 0x" << hex << address << endl;
      }
      else
      {
        cout << "Address: 0x" << hex << address << " out of bounds" << endl;
      }
    }/*EOF Read*/
    
    void write(unsigned int address, unsigned int data)
    {
       if (address < 250)
      {
        data_val[address] = data;
        cout << "Writing data " << hex << data << " @ address: 0x" << hex << address << endl;
      }
      else
      {
        cout << "Address: 0x" << hex << address << " out of bounds" << endl;
      }
    }/*EOF Write*/
};/* EOC Channel*/


class master : public sc_module
{
  public:
    sc_port<myInterface> out;
    SC_HAS_PROCESS(master);
    master(sc_module_name nm) : sc_module(nm)
    {
      SC_THREAD(master_Xfr);
    }
    void master_Xfr()
    {
       unsigned int w_data ;
    for(int i = 0; i < 10; i++) {
      w_data = i*3 ;
      out->write(i, w_data);
    }
    //Read Data
    unsigned int r_data ;
    for(int i = 0; i < 10; i++) {
      out->read(i, r_data);
    }
};

class top : public sc_module
{
  public:
    master master_inst("master_inst");
    channel channel_inst("channel_inst");
    top(sc_module_name nm) : sc_module(nm)
    {
      master_inst.out(channel);
    }
};

int sc_main(int argc, char** argv) {
  top tp_mod ("topModule");
  sc_start();
  return 0 ;
}
