// Ex4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
#include "CommitCounters.h"
#include <conio.h>

int main()
{
    std::cout << "!! Ex4: Committed Memory !!" << std::endl;

	std::string input = "";

	int pid = 0;

	while (true) {
		std::cout << "Please enter a valid process id: ";
		getline(std::cin, input);

		// This code converts from string to number safely.
		std::stringstream myStream(input);
		if (myStream >> pid)
		{
			const PCommitCounters counters = new CommitCounters();

			if(GetCommitCountersFromProcess(pid, _Out_ counters))
			{
				if(counters == nullptr)
				{
					std::cout << "something went wrong" << std::endl;
				}
				else
				{
					//print result
					printf("*** Allocated Commit Size ***\n");
					printf("\tImage: %dK\n", counters->img);
					printf("\tMapped: %dK\n", counters->map);
					printf("\tPrivate: %dK\n", counters->prv);
				}
			}

			continue;
		}

		if(input == "exit")
		{
			break;
		}

		std::cout << "Invalid number, please try again" << std::endl;
	}

	return 0;
}
