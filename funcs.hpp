/*
 * funcs.h
 *
 *  Created on: 24 нояб. 2016 г.
 *      Author: Dexiz
 */

#ifndef FUNCS_HPP_
#define FUNCS_HPP_

#include<iostream>
#include<fstream>
#include<vector>
#include<stdexcept>
#include<cstring>
#include<cstring>
//#include<cstdlib>
#include<cmath>
#include<limits>

#include<thread>
#include<mutex>
//#include"varDeclaration.h"
#include"semaphore.h"

class Task
{
	Semaphore sem;

	static const int BUFFER_SIZE = 94;
	//const int BUFFER_SIZE;

	//std::vector<bool> betaCoeficients;
	//unsigned long long int betaCoeficients;
	typedef unsigned long long int BetaCoef;
	std::vector<int> spectrum;
	//std::vector<std::mutex> spectrumMutex;
	std::mutex *spectrumMutexArray;

	std::mutex vectorsAccess;
	std::vector<unsigned int*> vectors; // Множество векторов А.
	int vectorSize; // Размер вектора N.
	int numberOfInts; // Для хранения вектора необходимо numberOfInts переменных типа int.

	//std::mutex filesAccess;
	std::ifstream input;
	std::ofstream output;

	bool vectorsLoaded;
	bool filesOpened;
	bool unixStyleLine;

public:
	Task();
	Task(std::string inputFile, std::string outputFile);
	void loadAllVectors();
	void freeVectors();
	void setInputOutputFiles(std::string inputFile, std::string outputFile);
	void computeSpectrumInParallel(unsigned int numberOfVectorsInOneThread);
	void writeSpectrumIntoOutputFile();
	~Task();

	//unsigned int* fetchVector(unsigned int i);

private:
	int removeDelimiterDetermineVectorSize(char CstyleString[]);
	int computeNumberOfInts(int vectorSize);
	void addVectorToVectors(std::string buffer);
	int computeWeight(unsigned int *pointer);
	void computeSpectrum(BetaCoef betaCoef, unsigned int numberOfVectorsInOneThread);
	bool getBit(BetaCoef betaCoef, int i);
};

#endif /* FUNCS_HPP_ */
