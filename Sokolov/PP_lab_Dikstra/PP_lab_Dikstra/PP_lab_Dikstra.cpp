// PP_lab_Dikstra.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>
#include "dikstra.h"
#include <ctime>

#define SIZE 5000

int main(int argc, char **argv)
{
	//creature
	dataType** Graph;
	dataType* Result;
	dataType* ResultMP;
	
	int min = 0;
	int max = 3;
	int SizeVertex = SIZE;

	srand(time(0));

	if (argc > 1)
		SizeVertex = atoi(argv[1]);
	if (argc > 2)
		max = atoi(argv[2]);
	if (argc > 3)
		min = atoi(argv[3]);


	Result = new dataType[SizeVertex];
	ResultMP = new dataType[SizeVertex];
	Graph = new dataType*[SizeVertex];
	for (int i = 0; i < SizeVertex; i++)
	{
		Graph[i] = new dataType[SizeVertex];
	}


	////initialization
	for (int i = 0; i < SizeVertex; i++)
	{
		for (int j = i; j < SizeVertex; j++)
		{
			if (i != j)
				Graph[i][j] = Graph[j][i] = (rand() % (max - min + 1) + min);
			else Graph[i][j] = NO_EBGE;
		}
	}
	//Graph[0][0] = 0;  Graph[0][1] = 7;  Graph[0][2] = 9;  Graph[0][3] = 0;  Graph[0][4] = 0; Graph[0][5] = 14;
	//Graph[1][0] = 7;  Graph[1][1] = 0;  Graph[1][2] = 10; Graph[1][3] = 15; Graph[1][4] = 0; Graph[1][5] = 0;
	//Graph[2][0] = 9;  Graph[2][1] = 10; Graph[2][2] = 0;  Graph[2][3] = 11; Graph[2][4] = 0; Graph[2][5] = 2;
	//Graph[3][0] = 0;  Graph[3][1] = 15; Graph[3][2] = 11; Graph[3][3] = 0;  Graph[3][4] = 6; Graph[3][5] = 0;
	//Graph[4][0] = 0;  Graph[4][1] = 0;  Graph[4][2] = 0;  Graph[4][3] = 6;  Graph[4][4] = 0; Graph[4][5] = 9;
	//Graph[5][0] = 14; Graph[5][1] = 0;  Graph[5][2] = 2;  Graph[5][3] = 0;  Graph[5][4] = 9; Graph[5][5] = 0;


	
	// 0 7 9 20 20 11
	double time_start, time_end, time_sequential;
	double time_start_MP, time_end_MP, time_parallel_MP;

	time_t ctime_start, ctime_end, ctime_sequential;
	time_t ctime_start_MP, ctime_end_MP, ctime_parallel_MP;
	//work
	time_start = omp_get_wtime();
	ctime_start = clock();
	dikstra(Graph, Result, SizeVertex);
	time_end = omp_get_wtime();
	ctime_end = clock();

	time_start_MP = omp_get_wtime();
	ctime_start_MP = clock();
	dikstraMP(Graph, ResultMP, SizeVertex);
	time_end_MP = omp_get_wtime();
	ctime_end_MP = clock();

	//cout << "Graph" << endl;

	//for (int i = 0; i < SizeVertex; i++)
	//{
	//	for (int j = 0; j < SizeVertex; j++)
	//	{
	//		cout << Graph[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << endl;

	//cout << "sequential algorithm" << endl;
	//for (int i = 0; i < SizeVertex; i++)
	//{
	//	cout << Result[i] << endl;
	//}
	//cout << endl;

	//cout << "parallel MP algorithm" << endl;
	//for (int i = 0; i < SizeVertex; i++)
	//{
	//	cout << ResultMP[i] << endl;
	//}
	//cout << endl;





	time_sequential = time_end - time_start;
	ctime_sequential = ctime_end - ctime_start;
	cout << "time sequential algorithm " << time_sequential << " "  << endl;

	time_parallel_MP = time_end_MP - time_start_MP;
	ctime_parallel_MP = ctime_end_MP - ctime_start_MP;
	cout << "time parallel MP algorithm " << time_parallel_MP << " "  << endl;

	dataType rez_sequential_and_MP = 0;
	for (int i = 0; i < SizeVertex; i++)
	{
		rez_sequential_and_MP += abs(Result[i] - ResultMP[i]);
	}
	cout << "control rezult sequential and MP " << rez_sequential_and_MP << endl;

	//dell
	delete[] Result;
	delete[] ResultMP;

	for (int i = 0; i < SizeVertex; i++)
	{
		delete[] Graph[i];
	}
	delete[] Graph;
}

