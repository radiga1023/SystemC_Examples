#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;

class simple_ext : public tlm::tlm_extension<simple_ext>
{
public:
	explicit simple_ext(const unsigned mid = 0,
	                    const unsigned tid = 0)
	{
		m_master_id = mid ;
		m_trans_id  = tid ;
	}
	unsigned get_master_id() const
	{
		return m_master_id ;
	}
	void set_master_id(const unsigned mid)
	{
		m_master_id = mid ;
	}
	unsigned get_trans_id() const
	{
		return m_trans_id ;
	}
	void set_trans_id(const unsigned tid)
	{
		m_trans_id = tid ;
	}
	//Madatory Function Clone
	virtual tlm_extension_base* clone() const
	{
		simple_ext* obj = new simple_ext(m_master_id, m_trans_id);
		return obj ;
	}
	//Madatory function copy_from
	virtual void copy_from(tlm_extension_base const &other)
	{
		m_master_id = static_cast<simple_ext const&>(other).get_master_id();
		m_trans_id  = static_cast<simple_ext const&>(other).get_trans_id();
	}
private:
	unsigned int m_master_id ;
	unsigned int m_trans_id  ;
};

class initiator : public sc_module
{
public:
	//Create a Socket
	tlm_utils::simple_initiator_socket<initiator> init_socket;
	tlm::tlm_generic_payload* trans;
	SC_HAS_PROCESS(initiator);
	initiator(sc_module_name name);
private:
	void thread_process();
      unsigned int transCount;
};
initiator::initiator(sc_module_name name)
		  : sc_module(name),
		    init_socket("InitSocket")
{
	SC_THREAD(thread_process);
	trans = new tlm::tlm_generic_payload;
}
void initiator::thread_process()
{
	simple_ext* ext = new simple_ext();
    transCount = 1 ;
//Write Operation
	for (unsigned int i = 0; i <= 20; i+= 4)
	{
		//Set the extension object
		ext->set_master_id(1);
		ext->set_trans_id(transCount++);
		sc_time delay = SC_ZERO_TIME;
		trans->set_write();
		trans->set_address(i);
		trans->set_data_length(4);
		trans->set_streaming_width(4);
		trans->set_byte_enable_ptr(0);
		trans->set_dmi_allowed(false);
		int data = 0xFF000000 | i ;
		trans->set_data_ptr((unsigned char*)(&data));
		
		trans->set_extension(ext);
		
		init_socket->b_transport(*trans, delay);
		if (trans->is_response_error())
			std::cout << "Response error from blocing transport" << std::endl;
		
		std::cout << "Write Transfer:- Address " << std::hex << i << ": Data:- " << std::hex << data
		<< std::endl;
		wait(1, SC_NS);
		//delete ext ;
	}
//Read Operation
	int r_data ;
     transCount = 1 ;
	for (unsigned int i = 0; i <= 20; i+= 4)
	{
		//Set the extension object
		ext->set_master_id(1);
		ext->set_trans_id(transCount++);
		sc_time delay = SC_ZERO_TIME;
		trans->set_read();
		trans->set_address(i);
		trans->set_data_length(4);
		trans->set_streaming_width(4);
		trans->set_byte_enable_ptr(0);
		trans->set_dmi_allowed(false);
		trans->set_data_ptr((unsigned char*)(&r_data));
		
		trans->set_extension(ext);
		
		init_socket->b_transport(*trans, delay);
		if (trans->is_response_error())
			std::cout << "Response error from blocing transport" << std::endl;
		
		std::cout << "Read Transfer:- Address " << std::hex << i << ": Data:- " << std::hex << r_data
		<< std::endl;
		wait(1, SC_NS);

	}
		delete ext ;
}

class target : public sc_module
{
public:
	tlm_utils::simple_target_socket<target> target_socket;
	target(sc_module_name name);
private:
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	int mem[256];
};
target::target(sc_module_name name) : sc_module(name), target_socket("Target_Socket")
{
	//Register Callbacks for forward transport method calls
	target_socket.register_b_transport(this, &target::b_transport);
	for (int i = 0; i < 256; i++) mem[i] = 0xAA000000 | i ;
}
void target::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64    adr = trans.get_address()/4;
	unsigned char*   ptr = trans.get_data_ptr();
	unsigned int     len = trans.get_data_length();
	unsigned int     wid = trans.get_streaming_width();
	unsigned char*   byt = trans.get_byte_enable_ptr();
	
	simple_ext* ext ;
	trans.get_extension(ext);
	
	std::cout << "@Target: Transaction ID:- " << std::hex << ext->get_trans_id() << " Master ID:- "
	<< ext->get_master_id() << std::endl ;
	
	if (cmd == tlm::TLM_READ_COMMAND)
		memcpy(ptr, &mem[adr], len);
	else if (cmd == tlm::TLM_WRITE_COMMAND)
		memcpy(&mem[adr], ptr, len);
	
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
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
	target* targ;
};

int sc_main(int argc, char** argv)
{
	top top1("TopModule");
	sc_start();
	return 0;
}

