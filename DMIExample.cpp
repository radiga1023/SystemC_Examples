#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
using namespace std;
using namespace sc_core;

class initiator : public sc_module
{
public:
	tlm_utils::simple_initiator_socket<initiator> init_socket;
	tlm::tlm_generic_payload* trans ;
	SC_HAS_PROCESS(initiator);
	initiator(sc_module_name name);
private:
	void thread_process();
	virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range);
    bool dmi_ptr_valid;
    tlm::tlm_dmi dmi_data;
};
initiator::initiator(sc_module_name name) 
		  : sc_module(name),
		    init_socket("InitSocket"), dmi_ptr_valid(false)
{
	SC_THREAD(thread_process);
	trans = new tlm::tlm_generic_payload;
	init_socket.register_invalidate_direct_mem_ptr(this, &initiator::invalidate_direct_mem_ptr);
}
void initiator::thread_process()
{
	int w_data, r_data;
	//Write
	for (int i = 0; i <= 20; i += 4)
	{
		if (dmi_ptr_valid)
		{
			assert(dmi_data.is_write_allowed());
			memcpy(dmi_data.get_dmi_ptr() + i, &w_data, 4);
			wait(dmi_data.get_write_latency());
			cout << "Write: DMI Address:- " << hex << i << " Data:- " << w_data << endl; 
		}
		else
		{
			trans->set_write();
			trans->set_address(i);
			trans->set_data_length(4);
			trans->set_streaming_width(4);
			trans->set_dmi_allowed(false);
			trans->set_byte_enable_ptr(0);
			w_data = 0x01 + i ;
			trans->set_data_ptr((unsigned char*) (&w_data));
			trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
			sc_time delay = SC_ZERO_TIME;
			init_socket->b_transport(*trans, delay);
			if (trans->is_response_error())
				cout << "Response error from b_transport" << endl;
            cout << "Write Trans : Address " << hex << i << " Data: " << w_data << endl;  
		}
		if (trans->is_dmi_allowed())
		{
			dmi_data.init();
			dmi_ptr_valid = init_socket->get_direct_mem_ptr(*trans, dmi_data) ;
		}
		wait(1, SC_NS);
	}
	//Read
	//int r_data ;
	for (int i = 0; i <= 20; i += 4)
	{
		if (dmi_ptr_valid)
		{
			assert(dmi_data.is_read_allowed());
			memcpy(&r_data, dmi_data.get_dmi_ptr() + i, 4);
			wait(dmi_data.get_read_latency());
			cout << "Read Op: DMI Address:- " << hex << i << " Data:- " << r_data << endl; 
		}
		else
		{
			trans->set_read();
			trans->set_address(i);
			trans->set_data_length(4);
			trans->set_streaming_width(4);
			trans->set_dmi_allowed(false);
			trans->set_byte_enable_ptr(0);
			w_data = 0x01 + i ;
			trans->set_data_ptr((unsigned char*) (&r_data));
			trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
			sc_time delay = SC_ZERO_TIME;
			init_socket->b_transport(*trans, delay);
			if (trans->is_response_error())
				cout << "Response error from b_transport" << endl;
            cout << "Read Trans : Address " << hex << i << " Data: " << r_data << endl;
		}
		if (trans->is_dmi_allowed())
		{
			dmi_data.init();
			dmi_ptr_valid = init_socket->get_direct_mem_ptr(*trans, dmi_data) ;
		}
		wait(1, SC_NS);
	}
}
void initiator::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range)
{
	dmi_ptr_valid = 0;
}

class target : public sc_module
{
public:
	enum e {SIZE=256};
	tlm_utils::simple_target_socket<target> target_socket;
	SC_HAS_PROCESS(target);
	target(sc_module_name name);
private:
	void invalidation_process(); 
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
	int mem[SIZE];
	tlm::tlm_dmi dmi_data;
};
target::target(sc_module_name name) : sc_module(name), target_socket("TargetSocket")
{
	SC_THREAD(invalidation_process);
	//Register call backs for forward interface method calls
	target_socket.register_b_transport(this, &target::b_transport);
	target_socket.register_get_direct_mem_ptr(this, &target::get_direct_mem_ptr);
    memset(mem, 0x0A, SIZE);
}
void target::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64    adr = trans.get_address();
	unsigned char*   ptr = trans.get_data_ptr();
	unsigned int     len = trans.get_data_length();
	unsigned char*   byt = trans.get_byte_enable_ptr();
	unsigned int     wid = trans.get_streaming_width();
	
	if (cmd == tlm::TLM_READ_COMMAND)
		memcpy(ptr, &mem[adr], len);
	else if (cmd == tlm::TLM_WRITE_COMMAND)
		memcpy(&mem[adr], ptr, len);
	
	wait(1, SC_NS);
	trans.set_dmi_allowed(true);
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
bool target::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data)
{
	dmi_data.allow_read_write();
	dmi_data.set_dmi_ptr((unsigned char*)(&mem[0]));
	dmi_data.set_start_address(0);
	dmi_data.set_end_address(SIZE*4 - 1);
	dmi_data.set_read_latency(sc_time(2, SC_NS));
	dmi_data.set_write_latency(sc_time(4, SC_NS));
	return true;
}
void target::invalidation_process()
{
	for (int i = 0; i < 5; i++)
	{
		target_socket->invalidate_direct_mem_ptr(0, (SIZE*4 -1 ) );
		wait(2, SC_NS);
	}
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
	top t1("TopMod");
	sc_start();
	return 0;
}
	     
