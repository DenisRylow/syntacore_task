/*
 * main.cpp
 *
 *  Created on: 24 םמÿב. 2016 ד.
 *      Author: Dexiz
 */

#include<iostream>
#include<fstream>
#include<vector>
#include<cstring>
#include<string>
#include<stdexcept>


#include "funcs.hpp"


int main()
{
	try
	{
		Task task("input1.txt","output.txt");
		task.loadAllVectors();
		task.computeSpectrumInParallel(2);
		task.writeSpectrumIntoOutputFile();
	}
	catch(std::runtime_error &err)
	{
		std::ofstream outputFile("log.txt", std::ios::out | std::ios::trunc); // Ëמד פאיכ.
		std::string errorText = err.what();
		errorText.append("\n") ;

		outputFile.write( errorText.c_str(), strlen(errorText.c_str()) );
	    outputFile.flush();
	    outputFile.close();

	    return 1;
	};


	return 0;
}
