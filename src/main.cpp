#include <iostream>
#include <new> 

#include "Project2Helper.h"
#include "tests.h"

// Pass 0-11 to choose the scenario
// IE: project2.exe 5
int main(int argc, char *argv[]) {
	int scenario = 3; // Or change this line to pick a scenario

	if (argc > 1) {
		scenario = std::atoi(argv[1]);
	}

	switch (scenario) {
		case 11: project2_vectornew_scalardelete(); break;
		case 10: project2_scalarnew_vectordelete(); break;
		case 9:  project2_randompointer1(); break;
		case 8:  project2_randompointer2(); break;
		case 7:  project2_doubledelete(); break;
		case 6:  project2_doublevectordelete(); break;
		case 5:  project2_deletedmemoryread(); break;
		case 4:  project2_deletedmemorywrite(); break;
		case 3:  project2_readoverflow(); break;
		case 2:  project2_writeoverflow(); break;
		case 1:  project2_leaks(); break;
		default: project2_good(); break;
	}

	return 0;
}
