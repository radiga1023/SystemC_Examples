//Debug Example Model

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace sc_core ;

class initiator : public sc_module
{
public:
	tlm_utils::simple_initiator_socket<initiator> init_socket;
	SC_HAS_PROCESS(initiator);
	tlm::tlm_generic_payload* trans;
	initiator(sc_module_name name);
private:
	void thread_process();
};

initiator::initiator(sc_module_name name) 
          : sc_module(name),
		    init_socket("Init_Socket")
{
	SC_THREAD(thread_process);
	trans = new tlm::tlm_generic_payload;
}
void initiator::thread_process()
{
	//Write
	for (int i = 0; i <= 5; i++)
	{
	  trans->set_write();
	  trans->set_address(i);
	  trans->set_data_length(1);
	  trans->set_streaming_width(1);
	  trans->set_dmi_allowed(false);
	  trans->set_byte_enable_ptr(0);
	  int data = 0x01 + i ;
	  trans->set_data_ptr((unsigned char*)(&data));
	  sc_time delay = SC_ZERO_TIME;
	  init_socket->b_transport(*trans, delay);
	  if (trans->is_response_error())
		cout << "Response error from b_transport" << endl;
	  unsigned n_bytes = init_socket->transport_dbg(*trans);
	  cout << "Debug Transport Called after Write blocking Trasfer" << endl;
	  cout << "------------------------------------------" << endl;
	  for (int j = 0; j < n_bytes; j++)
		{
			cout << "mem[" << j << "], Data = " << data << endl;
		}
	  
	  wait(2, SC_NS);
	}
	//Read Transfer
	int r_data ;
	for (int i = 0; i <= 5; i++)
	{
	  trans->set_read();
	  trans->set_address(i);
	  trans->set_data_length(1);
	  trans->set_streaming_width(1);
	  trans->set_dmi_allowed(false);
	  trans->set_byte_enable_ptr(0);
	  trans->set_data_ptr((unsigned char*)(&r_data));
	  sc_time delay = SC_ZERO_TIME;
	  init_socket->b_transport(*trans, delay);
	  if (trans->is_response_error())
		cout << "Response error from b_transport" << endl;
	  unsigned n_bytes = init_socket->transport_dbg(*trans);
	  cout << "Debug Transport Called after Read blocking Trasfer" << endl;
	  cout << "------------------------------------------" << endl;
	  for (int j = 0; j < n_bytes; j++)
		{
			cout << "mem[" << j << "], Data = " << r_data << endl;
		}
	  
	  wait(2, SC_NS);
	}
}

class target : public sc_module
{
public:
	tlm_utils::simple_target_socket<target> target_socket ;
	SC_HAS_PROCESS(target);
	target(sc_module_name name);
private:
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);
	int mem[256];
	int num_bytes_xfered;
};

target::target(sc_module_name name)
	   : sc_module(name),
	     target_socket("TargetSocket")
{
	target_socket.register_b_transport(this, &target::b_transport);
	target_socket.register_transport_dbg(this, &target::transport_dbg);
}
void target::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64    adr = trans.get_address();
	unsigned char*   ptr = trans.get_data_ptr();
	unsigned int     len = trans.get_data_length();
	unsigned int     wid = trans.get_streaming_width();
	unsigned char*   byt = trans.get_byte_enable_ptr();
	
	//if (cmd == tlm::TLM_READ_COMMAND)
	//memcpy(ptr, &mem[adr], len);
	//else if (cmd == tlm::TLM_WRITE_COMMAND)
	//memcpy(&mem[adr], ptr, len);
	wait(2, SC_NS);
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
unsigned int target::transport_dbg(tlm::tlm_generic_payload& trans)
{
	num_bytes_xfered = 0 ;
	sc_dt::uint64  address = trans.get_address();
	unsigned int   length  = trans.get_data_length();
	unsigned char* data_ptr= trans.get_data_ptr();
	tlm::tlm_command cmd   = trans.get_command();
	
	if (cmd == tlm::TLM_READ_COMMAND)
	{
		for(unsigned int i = 0; i < length; i++)
		{
			data_ptr[i] = mem[address++] ;
			//memcpy(data_ptr, &mem[address], length)
		    ++num_bytes_xfered ;
		}
	}
	else if (cmd == tlm::TLM_WRITE_COMMAND)
	{
		for(unsigned int i = 0; i < length; i++)
		{
			mem[address++]  = data_ptr[i]  ;
			//memcpy(data_ptr, &mem[address], length)
		    ++num_bytes_xfered ;
		}
	}
	return num_bytes_xfered;
}

class top : public sc_module
{
public:
	SC_CTOR(top)
	{
	init = new initiator("Initiator");
	targ = new target("Target");
	init->init_socket(targ->target_socket);
	}
private:
	initiator* init ;
	target* targ ;
};

int sc_main(int argc, char** argv)
{
	top t1("Top");
	sc_start();
	return 0;
}
