#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h> 

int main()
{
	SetConsoleTitle(L"ConsoleInterception"); //sets Window Title to Lose Health
	using namespace std::chrono_literals;
	const char* firstLine = "Hello";
	const char* secondLine = "Word";
	while (true)
	{
		std::cout << firstLine << "\n";
		std::cout << secondLine << "\n";
		std::this_thread::sleep_for(2s);
	}
}