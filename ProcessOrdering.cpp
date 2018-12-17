#include "systemc.h"
using namespace std;

class ThreadOrdering  : public sc_module
{
  public:
	SC_CTOR(ThreadOrdering)
	{
		SC_METHOD(methodOne) ;
		sensitive << terminateMethod_ev ;
		SC_THREAD(threadOne) ;
		SC_THREAD(threadTwo) ;
	}
	void threadOne()
	{
        SC_REPORT_INFO("TLM-2","Thread_1 Running");
		cout << "Thread One is Running...! @ " << sc_time_stamp()  << endl;
		//wait(1.0, SC_NS);
      my_ev.notify(20, SC_NS);
		cout << " Thread One Processing Done @ " << sc_time_stamp() << endl;
	}
	void threadTwo()
	{
      SC_REPORT_INFO("TLM-2","Thread_2 Running");
		cout << "Thread Two is Running...! @ " << sc_time_stamp() << endl;
		wait(my_ev);
		terminateMethod_ev.notify();
		cout << " Thread Two Processing Done @ " << sc_time_stamp() << endl;
	}
	void methodOne()
	{
		cout << "Method One Processing...!" <<" @ " << sc_time_stamp() << endl ;
        my_ev.notify(10, SC_NS);
		next_trigger(terminateMethod_ev);
	}
private:
	sc_event terminateMethod_ev, my_ev;
};

int sc_main(int argc, char** argv)
{
	ThreadOrdering tr ("ThreadOrdering");
	sc_start();
	return 0;
}
