#include "armProc/machine.h"

main(){
	cout << "TEST START, CREATING MACHINE..\n\n";
	machine* mac = new machine();
	cout << "\n INIT COMPLETED!\n\n----\n";
	processor* proc = mac->getCPU();
	if(proc != NULL)
		cout << "processor: exists\n";
	else
		cout << "processor: VOID\n";
	coprocessor *cop = mac->getCoprocessor(15);
	if(cop != NULL)
		cout << "coprocessor: exists\n";
	else
		cout << "coprocessor: VOID\n";
	cp15 *cp = (cp15*) cop;
	if(cp != NULL)
		cout << "cp15: castable\n";
	else
		cout << "cp15: VOID\n";
	coprocessor_interface *cpint = proc->getCopInt();
	if(cpint != NULL)
		cout << "coprocessor_interface: exists\n";
	else
		cout << "coprocessor_interface: VOID\n";
	
	systemBus *bus = mac->getBus();
	if(bus != NULL)
		cout << "systemBus: exists\n";
	else
		cout << "systemBus: VOID\n";
	
	ramMemory *ram = bus->getRam();
	if(ram != NULL)
		cout << "ramMemory: exists\n";
	else
		cout << "ramMemory: VOID\n";
	
	cout << "\nmac->bus: " << bus;
	cout << "\nproc->bus: " << proc->bus;
	cout << "\ncp15->bus: " << cp->bus;
	
	return 0;
}
