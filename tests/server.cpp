
#include <string>
#include <iostream>
#include <unistd.h>
#include "net.h"
#include "mm.h"
#include "common.h"
#include "layout.h"
#include "procs.h"

using namespace std;
using namespace LARM;
using namespace LARM::LARM_NET;
using namespace LARM::INLARM::LARM_MM;

const unsigned int MAX_RECORD = 100;
const unsigned int READ_RECORD = 95;

const unsigned int ITEM_CHECKS = 10000000;
const unsigned int NUM_READ = 10;


bool change_item();
bool add_attr();

int main(int argc, char * argv[]) {
	/*
	if (argc != 3) {
		cout << "Cannot get hostname and port" << endl; 
	}
	*/
	//string hostname(argv[1]);
	//string port(argv[2]);
	
	int  port = stoi(argv[1]);
	
	LARM::INLARM::LARM_MM::init_mem(20);
	start_net(port, (LARM::BYTE)get_base(), 20, processor, false);
	pthread_exit(NULL);

}


