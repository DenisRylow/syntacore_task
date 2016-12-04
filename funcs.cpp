/*
 * funcs.cpp
 *
 *  Created on: 25 нояб. 2016 г.
 *      Author: Dexiz
 */
#include"funcs.hpp"

void Task::setInputOutputFiles(std::string inputFile, std::string outputFile)
{
	input.open(inputFile, std::ios::in);  // Входной файл.
	output.open(outputFile, std::ios::out | std::ios::trunc); // Выходной файл.

	if (input.fail() || output.fail())
	{
		// Ошибка открытия файла.
		throw std::runtime_error("Could not open one or both files.");
	}
	else
		if (input.peek() == std::fstream::traits_type::eof())
		{
			// Файл пустой.
			throw std::runtime_error("Empty input file.");
		};
}


Task::Task()
{
	numberOfInts = 0;
	vectorSize = 0;

	vectorsLoaded = false;
	filesOpened = false;
	unixStyleLine = true;
}

Task::Task(std::string inputFile, std::string outputFile)
{
	setInputOutputFiles(inputFile, outputFile);

	numberOfInts = 0;
	vectorSize = 0;

	vectorsLoaded = false;
	filesOpened = true;
	unixStyleLine = true;

	loadAllVectors();
}


int Task::removeDelimiterDetermineVectorSize(char CstyleString[])
{
	int size = strlen(CstyleString);

	if (CstyleString[size - 1] == '\r') // Windows-style строки заканчиваются на \r\n, \r необходимо убрать.
	{
		unixStyleLine = false;
		CstyleString[size - 1] = '\0'; // Убираем '\r'.
		--size;
	};

	return size;
}


int Task::computeNumberOfInts(int vectorSize)
{
	int m = vectorSize % std::numeric_limits<unsigned int>::digits;
	int n = vectorSize / std::numeric_limits<unsigned int>::digits;

	if (m != 0)
		++n;

	return n;
}


void Task::addVectorToVectors(std::string buffer)
{
	unsigned int *pointer = new unsigned int[numberOfInts];

	vectorsAccess.lock();
	vectors.push_back(pointer);
	vectorsAccess.unlock();

	int i = 0;
	int j = 0;
	*(pointer + j) = 0;

	while(buffer[i] != '\0')
	{
		switch(buffer[i])
		{
			case '0':
				*(pointer + j) = *(pointer + j) << 1;
				break;
			case '1':
				*(pointer + j) = (*(pointer + j) << 1) + 1;
				break;
		};

		bool b = (i + 1) % std::numeric_limits<unsigned int>::digits;
		if (!b)
		{
			// Текущая ячейка памяти заполнена, необходимо перейти на следующую.
			++j;
			*(pointer + j) = 0;
		};

		++i;
	};

	sem.signalWorkFinished();
}

/*
unsigned int* Task::fetchVector(unsigned int i)
{
	unsigned int *pointer;

	if (vectors.size() > i )
	{
		pointer = vectors[0];
	}
	else
	{
		vectorsAccess.lock();
		pointer = vectors[i];
		vectorsAccess.unlock();
	};

	return pointer;
}
*/

void Task::loadAllVectors()
{
	if (!vectorsLoaded && filesOpened)
	{
		char buffer[BUFFER_SIZE];
		int currentVectorSize;

		freeVectors();

		input.getline(buffer,BUFFER_SIZE);

		vectorSize = removeDelimiterDetermineVectorSize(buffer); // Число N.
		numberOfInts = computeNumberOfInts(vectorSize);

		vectors.reserve(16);
		spectrum.clear();
		spectrum.resize(vectorSize + 1);
		spectrumMutexArray = new std::mutex[vectorSize + 1];

		sem.signalWorkStarted();
		addVectorToVectors(buffer);

		bool terminate = false;
		while (!input.eof() & !terminate)
		{
			input.getline(buffer,BUFFER_SIZE);
			currentVectorSize = removeDelimiterDetermineVectorSize(buffer);

			if (currentVectorSize != vectorSize)
			{
				// Все вектора должны быть одинакового размера
				terminate = true;
				//throw std::runtime_error("Vectors are of different sizes.");
			}
			else
			{
				sem.signalWorkStarted();
				std::thread thr(&Task::addVectorToVectors, this, buffer);
				thr.detach();
			};

		}

		sem.wait(); // Ожидание окончания работы всех std::thread.
		vectorsLoaded = true;
	};
}

void Task::computeSpectrumInParallel(unsigned int numberOfSumVectorsInOneThread)
{
	if (vectorsLoaded)
	{
		BetaCoef limit = 1 << vectors.size();

		if(numberOfSumVectorsInOneThread == 0 || numberOfSumVectorsInOneThread >= limit)
		{
			freeVectors();
			throw std::runtime_error("numberOfSumVectorsInOneThread is equal or less than zero or exceeds 2^{the number of generating vectors}.");
		};

		BetaCoef betaCoeficients = 0; // betaCoeficients - массив из N коэффициентов бета, на которые умнажаются порождающие вектора.

		while(betaCoeficients + numberOfSumVectorsInOneThread < limit)
		{
			sem.signalWorkStarted();
			// В каждом потоке обрабатывается numberOfSumVectorsInOneThread векторов - линейных сумм.
			std::thread thr(&Task::computeSpectrum, this, betaCoeficients, numberOfSumVectorsInOneThread);
			thr.detach();
			//thr.join();

			betaCoeficients = betaCoeficients + numberOfSumVectorsInOneThread;
		};

		// В последнем потоке может обрабатываться меньшее количество сумм векторов (не хватило).
		BetaCoef temp = limit - betaCoeficients;
		unsigned int numberOfVectorsInTheLastThread = temp;

		if(numberOfVectorsInTheLastThread != 0)
		{
			sem.signalWorkStarted();
			computeSpectrum(betaCoeficients, numberOfVectorsInTheLastThread);
		};

		sem.wait(); // Ожидание окончания работы всех std::thread.
	};

}

void Task::computeSpectrum(BetaCoef betaCoef, unsigned int numberOfVectorsInOneThread)
{
	int weight;
	unsigned int *p = new unsigned int[vectorSize];

	for(unsigned int i = 0; i < numberOfVectorsInOneThread; ++i)
	{
		// Обнуление вектора, который содержит линейную сумму.
		for(int j = 0; j < vectorSize; ++j)
			p[j] = 0;

		// Обнуление веса вектора p.
		weight = 0;

		for(unsigned int j = 0; j < vectors.size(); ++j) // vectors.size() = N
		{
			if (getBit(betaCoef, j)) // Если бит нулевой, то можно не складывать.
			{
				for(int k = 0; k < numberOfInts; ++k)
					p[k] ^= vectors[j][k];
			};
		};

		weight = computeWeight(p);

		spectrumMutexArray[weight].lock();
		++spectrum[weight];
		spectrumMutexArray[weight].unlock();

		++betaCoef;
	};

	sem.signalWorkFinished();
	delete [] p;
}


int Task::computeWeight(unsigned int *pointer)
{
	int sum = 0;
	int j = 0;

	while (j < numberOfInts)
	{
		sum += __builtin_popcount(pointer[j]); // Функция компилятора GCC.
		++j;
	}

	return sum;
}


bool Task::getBit(BetaCoef betaCoef, int i)
{
	bool b = (betaCoef >> i) & 1;
	return b;
}

void Task::writeSpectrumIntoOutputFile()
{
	char buff[20];
	char result[50];

	for(unsigned int i = 0; i < spectrum.size(); ++i)
	{
		sprintf(buff,"%d",i);
		strcpy(result,buff);
		strcat(result,"\t");

		sprintf(buff,"%d",spectrum[i]);
		strcat(result,buff);

		if (unixStyleLine)
			strcat(result,"\n");
		else
			strcat(result,"\r\n");

		output.write(result,strlen(result));
		//output.write("\r\n",strlen("\r\n"));
	}
}

void Task::freeVectors()
{
	if(vectorsLoaded)
	{
		for(unsigned int* c: vectors)
			delete[] c;

		delete [] spectrumMutexArray;

		vectorsLoaded = false;
	};

	vectors.clear();
}

Task::~Task()
{
	freeVectors();
}
