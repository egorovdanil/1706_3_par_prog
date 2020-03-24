// MPI_lab.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"

#include "mpi.h"
#include <iostream>
#include <windows.h>
#include <ctime>
#include <queue>

#define NO_EDGE 0

using namespace std;

struct Too_Int
{
	int value;
	int proc;
	bool operator == (const Too_Int &B)
	{
		if (this->value == B.value)
			if (this->proc == B.proc)
				return true;
		return false;
	}
};

//19. Поиск кратчайших путей из одной вершины (алгоритм Дейкстры).
int main(int argc, char **argv)
{
	srand(time(0));

	int ProcNum, ProcRank;
	MPI_Init(&argc, &argv);
	MPI_Status status;

	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	
	//Начало
		//Чтение данных		
		//Матрицы M x N (M - строк, N - столбцов)
	    int SizeVertex = 1000;

		int min = 0;
		int max = 1;

		if (argc > 1)
	    	SizeVertex = atoi(argv[1]);
		if (argc > 2)
			max = atoi(argv[2]);
		if (argc > 3)
			min = atoi(argv[3]);
		
		
		////////////////////////////////////////
		int  MAX_PROCCES = SizeVertex;


		MPI_Group UsingProcces;
		MPI_Comm_group(MPI_COMM_WORLD, &UsingProcces);

		int usingProcces;
		if (ProcNum > MAX_PROCCES)
			usingProcces = MAX_PROCCES;
		else usingProcces = ProcNum;

		int* ArrUsingProcces = new int[usingProcces];
		for (int i = 0; i < usingProcces; i++)
		{
			ArrUsingProcces[i] = i;
		}
		MPI_Group_incl(UsingProcces, usingProcces, ArrUsingProcces, &UsingProcces);
		delete[] ArrUsingProcces;


		MPI_Comm Using;
		MPI_Comm_create(MPI_COMM_WORLD, UsingProcces, &Using);

		////////////////////////////////

		if (ProcRank < usingProcces)
		{

		int* loadProcces = new int[usingProcces];

		for (int i = 0; i < usingProcces; i++)
		{
			loadProcces[i] = SizeVertex / usingProcces;
		}
		//нагрузить процессы, отличные от 0 сильнее
		for (int i = 0; i < (SizeVertex % usingProcces); i++)
		{
			loadProcces[i]++;
		}

		int* indexWork = new int[usingProcces];
		indexWork[0] = 0;
		for (int i = 1; i < usingProcces; i++)
		{
			indexWork[i] = indexWork[i - 1] + loadProcces[i - 1];
		}

			int* RezDist = new int[loadProcces[ProcRank]];
			int* ArrBuff = new int[loadProcces[ProcRank]];
			bool* Mark = new bool[loadProcces[ProcRank]];

			Too_Int local_min;
			Too_Int global_min;


			//НУЛЕВОЙ
			if (ProcRank == 0)
			{
				//Инициализация
				int** MatrixAdjacency = new int*[SizeVertex];
				for (int i = 0; i < SizeVertex; i++)
				{ 
					MatrixAdjacency[i] = new int[SizeVertex];
				}

				for (int i = 0; i < SizeVertex; i++)
				{
					for (int j = i; j < SizeVertex; j++)
					{
						if (i != j) 
							MatrixAdjacency[i][j] = MatrixAdjacency[j][i] = (rand() % (max - min + 1) + min) ;
						else MatrixAdjacency[i][j] = NO_EDGE;
					}
				}

			 // NO_EDGE - нет ребра
				 
				//print Matrix incedent

				//for (int i = 0; i < SizeVertex; i++)
				//{
				//	for (int j = 0; j < SizeVertex; j++)
				//	{
				//		cout << MatrixAdjacency[0][j] << " ";
				//	}
				//	cout << "\n";
				//}
				//cout << "\n";

				//////////////////////////////////////////////////////паралельн алгоритм ///////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				double start_paralel_time = MPI_Wtime();

				int* Dist = new int[SizeVertex];
				
		     	for (int i = 0; i < loadProcces[ProcRank]; i++)
				{
					RezDist[i] = INT_MAX;
					Mark[i] = false;
				}
				RezDist[0] = 0;
				
				bool global_flag = true;
				while (global_flag)
				{
					//поиск минимального не помеченного

					local_min.value = INT_MAX;
					local_min.proc = INT_MAX;

					for (int i = 0; i < loadProcces[ProcRank]; i++)    
					{
						if (Mark[i] == false)
							if (RezDist[i] <= local_min.value)
							{
								local_min.value = RezDist[i];
								local_min.proc = i + indexWork[ProcRank];
							}
					}
					MPI_Allreduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, Using);

					if (global_min == local_min)
						Mark[local_min.proc - indexWork[ProcRank]] = true;
					

					//перекидка всем нужной строки
					MPI_Scatterv(MatrixAdjacency[global_min.proc], loadProcces, indexWork, MPI_INT, ArrBuff, loadProcces[ProcRank], MPI_INT, 0, Using);
					//пересчёт дистанций
					for (int i = 0; i < loadProcces[ProcRank]; i++)                                                          
					{
						if (Mark[i] == false)
							if (ArrBuff[i] != NO_EDGE)
								RezDist[i] = min(RezDist[i], global_min.value + ArrBuff[i]);
					}
				
					// проверка на работу
					
					bool local_flag = false;
					for (int i = 0; i < loadProcces[ProcRank]; i++)                                          
					{
						if (Mark[i] == false)
							local_flag = true;
					}
					MPI_Allreduce(&local_flag, &global_flag, 1, MPI_BYTE, MPI_BOR, Using);

				}

				MPI_Gatherv(RezDist, loadProcces[ProcRank], MPI_INT, Dist, loadProcces, indexWork, MPI_INT, 0 ,Using);
				
				double end_paralel_time = MPI_Wtime();
				

				/*cout << "parralel algorithm " << "\n";
				for (int i = 0; i < SizeVertex; i++)
				{
					cout << Dist[i] << " ";
				}
				cout << endl;

*/



				cout << "time paralel = " << (end_paralel_time - start_paralel_time) << endl;

				////////////////////////////////////////////////однопоточный алгоритм/////////////////////////////////////////////////////


				double start_sequential_time = MPI_Wtime();
				

				int* Dist2 = new int[SizeVertex];
				bool* passedVertex2 = new bool[SizeVertex];
				for (int i = 1; i < SizeVertex; i++)
				{
					passedVertex2[i] = false;
					Dist2[i] = INT_MAX;
				}
				passedVertex2[0] = false;
				Dist2[0] = 0;

				int temp;
				int index2;
				int min_2;
				
				bool flagWork = true;
				while (flagWork)
				{
					min_2 = INT_MAX;
					for (int i = 0; i < SizeVertex; i++)                                                         
					{
						if (passedVertex2[i] == false)
							if (Dist2[i] <= min_2)
							{
								min_2 = Dist2[i];
								index2 = i;
							}
					}
					passedVertex2[index2] = true;

					for (int i = 0; i < SizeVertex; i++)                                                         
					{
						if (!passedVertex2[i])
							if(MatrixAdjacency[index2][i] != NO_EDGE)
							Dist2[i] = min(Dist2[i], Dist2[index2] + MatrixAdjacency[index2][i]);
					}

					flagWork = false;
					for (int i = 0; i < SizeVertex; i++)                                        					
					{
						if (passedVertex2[i] == false)
							flagWork = true;
					}
				}





				delete[] passedVertex2;
               
				double end_sequential_time = MPI_Wtime();

				/*cout << "sequential algorithm " << "\n";
				for (int i = 0; i < SizeVertex; i++)
				{
					cout << Dist2[i] << " ";
				}
				cout << endl;
*/
				int k = 0;
				for (int i = 0; i < SizeVertex; i++)
				{
					k+= Dist2[i] - Dist[i];
				}

				cout << "\ntime sequential = " << (end_sequential_time - start_sequential_time) << endl;
				cout << "\n\nA1-A2 = " << k << endl;

				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				delete[] Dist;
				delete[] Dist2;

				for (int i = 0; i < SizeVertex; i++)
				{
					delete[] MatrixAdjacency[i];
				}
				delete[] MatrixAdjacency;

			}
			//работа других процессов
			else
			{
				for (int i = 0; i < loadProcces[ProcRank]; i++)
				{
					RezDist[i] = INT_MAX;
					Mark[i] = false;
				}
				bool global_flag = true;

				while (global_flag)
				{
					//поиск минимального не помеченного

					local_min.value = INT_MAX;
					local_min.proc = INT_MAX;

					for (int i = 0; i < loadProcces[ProcRank]; i++)    
					{
						if (Mark[i] == false)
							if (RezDist[i] <= local_min.value)
							{
								local_min.value = RezDist[i];
								local_min.proc = i + indexWork[ProcRank];
							}
					}
					MPI_Allreduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, Using);

					if (global_min == local_min)
						Mark[local_min.proc - indexWork[ProcRank]] = true;


					//перекидка всем нужной строки
					MPI_Scatterv(NULL, loadProcces, indexWork, MPI_INT, ArrBuff, loadProcces[ProcRank], MPI_INT, 0, Using);
					//пересчёт дистанций
					for (int i = 0; i < loadProcces[ProcRank]; i++)                                                         
					{
						if (Mark[i] == false)
							if (ArrBuff[i] != NO_EDGE)
								RezDist[i] = min(RezDist[i], global_min.value + ArrBuff[i]);
					}

					// проверка на работу

					bool local_flag = false;
					for (int i = 0; i < loadProcces[ProcRank]; i++)                                         
					{
						if (Mark[i] == false)
							local_flag = true;
					}
					MPI_Allreduce(&local_flag, &global_flag, 1, MPI_BYTE, MPI_BOR, Using);

				}

				MPI_Gatherv(RezDist, loadProcces[ProcRank], MPI_INT, NULL, loadProcces, indexWork, MPI_INT, 0, Using);

			}

			//общая часть


			delete[] loadProcces;
			delete[] indexWork;
			delete[] RezDist;
			delete[] ArrBuff;
			delete[] Mark;

		}
	MPI_Group_free(&UsingProcces);
	MPI_Comm_free(&Using);
	MPI_Finalize();


	return 0;
}
