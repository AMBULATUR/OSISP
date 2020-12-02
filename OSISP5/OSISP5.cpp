#include <iostream>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <string>
#include <functional>
#include <fstream>

int numThreads;
std::string filePath;
std::string filePath_to_save;
static std::vector<std::string>** sorted_string_arrs;
static int countEl;

HANDLE hMutex;

/// <summary>
/// REFERENCE FOR EACH ELEM
/// </summary>
class Element
{
public:

	Element(std::vector<std::string> str_Arr)
	{
		this->str_Arr = str_Arr;
	}

	std::vector<std::string>* getstringArr()
	{
		return &this->str_Arr;
	}
private:
	std::vector<std::string> str_Arr;
};

/// <summary>
/// Очередь заданий
/// </summary>
class Queue
{
public:

	Queue()
	{
		isAdditing = 0;
		this->countEl = 0;
	}
	/// <summary>
	/// Добавление в очередь
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	int addToQueue(Element element)
	{
		WaitForSingleObject(hMutex, INFINITE);
		while (isAdditing)
			Sleep(100);
		isAdditing = 1;
		Arr_elements.push_back(element);
		this->countEl += 1;
		isAdditing = 0;
		ReleaseMutex(hMutex);
		return this->countEl;
	}

	int getCountEl()
	{
		return this->countEl;
	}

	Element getElement()
	{
		while (isAdditing)
			Sleep(10);
		isAdditing = 1;
		Element Temp = Arr_elements.back();
		Arr_elements.pop_back();
		this->countEl--;
		isAdditing = 0;
		return Temp;
	}

private:
	int isAdditing;
	std::vector<Element> Arr_elements;
	int countEl;
};



/// <summary>
/// Each array for each thread in a row
/// </summary>
class Arr_id
{
public:
	std::vector<std::string> vector;
	Arr_id(int id, std::vector<std::string> vector)
	{
		this->id = id;
		this->vector = vector;
	}

	int getId()
	{
		return this->id;
	}

	std::vector<std::string>* getVector()
	{
		return &(this->vector);
	}

private:
	int id;
};


Arr_id* arr_id;

/// <summary>
/// Func for new thread
/// включая Alg sort
/// </summary>
/// <param name="lParam"></param>
/// <returns></returns>
DWORD WINAPI sort_func(LPVOID lParam)
{
	Arr_id* arr_id = ((Arr_id*)lParam);
	int id = arr_id->getId();
	std::vector<std::string>* vector = arr_id->getVector();
	std::sort((*vector).begin(), (*vector).end());
	sorted_string_arrs[id] = vector;
	
	return 0;
}

class ThreadsApi
{
public:
	HANDLE* Threads;

	ThreadsApi(int numThreads)
	{
		this->Threads = new HANDLE[numThreads];
		this->numThreads = numThreads;
	}

private:
	int numActiv = 0;
	int numThreads;
};


/// <summary>
/// Раскидываем очередь по потокам
/// </summary>
class QueueWorker
{
public:
	QueueWorker(int numThreads, Queue queue)
	{
		this->queue = queue;
		this->numThreads = numThreads;
		this->Threads = new ThreadsApi(numThreads);
		this->countEl = queue.getCountEl();
	}

	void startQueue()
	{
		int index = 0;
		int thread_state;

		while (this->countEl-- > 0)
		{
			Element element = this->queue.getElement();
			arr_id = new Arr_id(index, *element.getstringArr());

			this->Threads->Threads[index++] = CreateThread(NULL,
				0,
				(LPTHREAD_START_ROUTINE)sort_func,
				arr_id,
				0,
				NULL);
		}
		WaitForMultipleObjects(numThreads, (const HANDLE*)this->Threads->Threads, TRUE, INFINITE);
		
	}

private:
	ThreadsApi* Threads;
	Queue queue;
	int numThreads;
	int countEl;
};

QueueWorker* queueWorker;
std::vector<std::string> string_vector;
std::vector<std::string> sorted_string_vector;
Queue queue;

/// <summary>
/// объединяет их методом сортирующего слияния.
/// </summary>
/// <param name="Arr"></param>
/// <param name="num"></param>
/// <returns></returns>
std::vector<std::string> merge(std::vector<std::string>** Arr, int num)
{
	int arr_len = 0; int key; int Num_Min = 0;
	for (int i = 0; i < num; i++)
	{
		arr_len += (*(Arr[i])).size();
	}

	std::vector<std::string> Arr_merged(arr_len);
	int* indexes = new int[num];
	for (int i = 0; i < num; i++)
		indexes[i] = 0;
	std::string Min;
	for (int i = 0; i < arr_len; i++)
	{
		for (Num_Min = 0; Num_Min < num; Num_Min++)
		{
			if (indexes[Num_Min] < (*Arr[Num_Min]).size())
			{
				Min = (*Arr[Num_Min])[indexes[Num_Min]];
				break;
			}
		}
		for (key = 0; key < num; key++)
		{
			if ((indexes[key] < (*Arr[key]).size()) && (Min > (*Arr[key])[indexes[key]]))
			{
				Num_Min = key;
			}
		}
		Arr_merged[i] = (*Arr[Num_Min])[indexes[Num_Min]];
		//Слияние массивов##################
		indexes[Num_Min]++;
	}
	return Arr_merged;
}
/// <summary>
/// 4. сортируют свои части файла
/// </summary>
void unionSortesStrings()
{
	int ArrLen = 0;
	for (int i = 0; i < numThreads; i++)
	{
		ArrLen += (*sorted_string_arrs[i]).size();
	
		std::cout << "Received sorted vector number #" << i << std::endl;
		for (int j = 0; j < (*sorted_string_arrs[i]).size(); j++)
		{
			std::cout << (*sorted_string_arrs[i])[j] << std::endl;
		}
		std::cout << "########################" << std::endl;
	
	}

	sorted_string_vector.reserve(ArrLen);


	for (int i = 0; i < numThreads; i++)
	{

		sorted_string_vector.insert((sorted_string_vector).end(),
			(*sorted_string_arrs[i]).begin(),
			(*sorted_string_arrs[i]).end());
	}
}
/// <summary>
/// 3. Сортирующие потоки извлекают задания,
/// </summary>
void startSorting()
{
	sorted_string_arrs = new std::vector<std::string>*[numThreads];
	queueWorker = new QueueWorker(numThreads, queue); 
	queueWorker->startQueue();
	unionSortesStrings();
	sorted_string_vector = merge(sorted_string_arrs, numThreads);
	return;
}
/// <summary>
/// Split each line to string vector
///1.  Входной поток читает файл в память
/// </summary>
/// <param name="filePath">Path to read file</param>
void readFile(std::string filePath)
{
	std::ifstream file;
	file.open(filePath);
	if (!file.is_open() || !file.good())
		return;
	std::string line;

	std::cout << "Elements from File: " << std::endl;
	while (std::getline(file, line))
	{
		string_vector.push_back(line);
		std::cout << line << std::endl;
	}
	std::cout << "Read from file success" << std::endl;
	return;
}
/// <summary>
/// Make Queue for threads
/// 2. нарезает его на части и создает 
/// несколько заданий на сортировку (по числу сортирующих потоков)
/// которые помещает в очередь заданий
/// </summary>
void makeQueue()
{
	int len = string_vector.size() / numThreads;
	std::vector<std::vector<std::string>> Arr;
	for (int i = 0; i < numThreads - 1; i++)
	{
		Arr.push_back(std::vector<std::string>(string_vector.begin() + i * len,
			string_vector.begin() + (i + 1) * len));
	}


	Arr.push_back(std::vector<std::string>(string_vector.begin() + (numThreads - 1) * len,
		string_vector.end()));
	for (int i = 0; i < numThreads; i++)
	{
		std::cout << "Not sorted elements in thread #" << i << std::endl;
		for (int j = 0; j < Arr[i].size(); j++)
			std::cout << Arr[i].at(j) << std::endl;
		std::cout << "#################################" << std::endl;
		countEl++;
		queue.addToQueue(Element(Arr[i]));
	}
}

int saveToFile(std::string filePath_to_save)
{
	std::ofstream file;
	file.open(filePath_to_save);
	if (!file.is_open())
	{
		return -1;
	}
	std::cout << "Merged and sorted array" << std::endl;
	for (int i = 0; i < sorted_string_vector.size(); i++)
	{
		std::cout << sorted_string_vector[i] << std::endl;
		file << sorted_string_vector[i] << "\n";
	}

	return 0;
}

int main()
{
	hMutex = CreateMutex(NULL, FALSE, NULL);
	numThreads = 2;
	filePath = "D:\\1.txt";
	filePath_to_save = "D:\\2.txt";

	readFile(filePath);
	makeQueue();
	startSorting();
	saveToFile(filePath_to_save);
	std::cout << "Done" << std::endl;

}