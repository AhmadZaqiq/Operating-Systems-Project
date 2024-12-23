//Ahmad JR
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

const string FileName = "Processes.txt";

struct stProcess
{
	string Name = "";
	int ArrivalTime = 0;
	int BurstTime = 0;
	int RemainingTime = 0;
	int FinishTime = 0;
	int WaitingTime = 0;
	int TurnaroundTime = 0;
};

struct stGanttSegment
{
	string ProcessName = "";
	int StartTime = 0;
	int EndTime = 0;
};

int ReadNumber(string Message)
{
	int Number = 0;

	do
	{
		cout << Message;
		cin >> Number;

	} while (Number < 1 || Number>4);

	return Number;
}

vector<string> SplitString(string S1, string delim)
{
	vector<string> vString;
	size_t pos = 0;
	string token;

	while ((pos = S1.find(delim)) != string::npos)
	{
		token = S1.substr(0, pos);
		if (!token.empty())
		{
			vString.push_back(token);
		}
		S1.erase(0, pos + delim.length());
	}

	if (!S1.empty())
	{
		vString.push_back(S1);
	}
	return vString;
}

stProcess ConvertLineToProcess(string Line)
{
	stProcess Process;
	vector<string> vProcessData = SplitString(Line, "#//#");

	if (vProcessData.size() >= 3)
	{
		Process.Name = vProcessData[0];
		Process.ArrivalTime = stoi(vProcessData[1]);
		Process.BurstTime = stoi(vProcessData[2]);
	}

	return Process;
}

vector<stProcess> LoadProcessesDataFromFile(string FileName)
{
	fstream MyFile;
	vector<stProcess> vProcesses;

	MyFile.open(FileName, ios::in);
	if (MyFile.is_open())
	{
		string Line;
		while (getline(MyFile, Line))
		{
			if (Line.find("Q=") == string::npos)
			{
				vProcesses.push_back(ConvertLineToProcess(Line));
			}
		}
		MyFile.close();
	}
	else
	{
		cout << "Error" << endl;
	}

	return vProcesses;
}

int ReadQuantumFromFile(string FileName)
{
	fstream MyFile;
	MyFile.open(FileName, ios::in);

	int Quantum = 0;
	if (MyFile.is_open())
	{
		string Line;
		while (getline(MyFile, Line))
		{
			if (Line.find("Q=") != string::npos)
			{
				Quantum = stoi(Line.substr(2));
				break;
			}
		}
		MyFile.close();
	}
	else
	{
		cout << "Error" << endl;
	}

	return Quantum;
}

void UpdateCurrentTime(int& CurrentTime, int ArrivalTime)
{
	if (CurrentTime < ArrivalTime)
	{
		CurrentTime = ArrivalTime;
	}
}

int CalculateWaitingTime(int CurrentTime, int ArrivalTime)
{
	return CurrentTime - ArrivalTime;
}

int CalculateFinishTime(int CurrentTime, int BurstTime)
{
	return CurrentTime + BurstTime;
}

int CalculateTurnaroundTime(int FinishTime, int ArrivalTime)
{
	return FinishTime - ArrivalTime;
}

void FirstComeFirstServed(vector<stProcess>& vProcesses, vector<stGanttSegment>& vGanttChart)
{
	int CurrentTime = 0;

	for (stProcess& P : vProcesses)
	{
		UpdateCurrentTime(CurrentTime, P.ArrivalTime);

		stGanttSegment segment;
		segment.ProcessName = P.Name;
		segment.StartTime = CurrentTime;

		P.WaitingTime = CalculateWaitingTime(CurrentTime, P.ArrivalTime);
		P.FinishTime = CalculateFinishTime(CurrentTime, P.BurstTime);
		CurrentTime = P.FinishTime;

		P.TurnaroundTime = CalculateTurnaroundTime(P.FinishTime, P.ArrivalTime);

		segment.EndTime = CurrentTime;
		vGanttChart.push_back(segment);
	}
}

void ShortestJobFirstPreemptive(vector<stProcess>& vProcesses, vector<stGanttSegment>& vGanttChart)
{
	int CurrentTime = 0;
	vector<int> vRemainingBurstTime(vProcesses.size());
	int Complete = 0, Minimum = INT_MAX, Shortest = -1;
	bool Check = false;

	for (int i = 0; i < vProcesses.size(); i++)
	{
		vRemainingBurstTime[i] = vProcesses[i].BurstTime;
	}

	while (Complete != vProcesses.size())
	{
		Minimum = INT_MAX;
		Shortest = -1;
		Check = false;

		for (int j = 0; j < vProcesses.size(); j++)
		{
			if ((vProcesses[j].ArrivalTime <= CurrentTime) &&
				(vRemainingBurstTime[j] < Minimum) &&
				vRemainingBurstTime[j] > 0)
			{
				Minimum = vRemainingBurstTime[j];
				Shortest = j;
				Check = true;
			}
		}

		if (!Check)
		{
			CurrentTime++;
			continue;
		}

		if (!vGanttChart.empty() && vGanttChart.back().ProcessName == vProcesses[Shortest].Name)
		{
			vGanttChart.back().EndTime = CurrentTime + 1;
		}
		else
		{
			vGanttChart.push_back({ vProcesses[Shortest].Name, CurrentTime, CurrentTime + 1 });
		}

		vRemainingBurstTime[Shortest]--;

		if (vRemainingBurstTime[Shortest] == 0)
		{
			Complete++;
			Check = false;

			vProcesses[Shortest].FinishTime = CurrentTime + 1;
			vProcesses[Shortest].WaitingTime = CalculateWaitingTime
			(
				(vProcesses[Shortest].FinishTime - vProcesses[Shortest].BurstTime),
				vProcesses[Shortest].ArrivalTime
			);
			vProcesses[Shortest].TurnaroundTime = CalculateTurnaroundTime
			(
				vProcesses[Shortest].FinishTime,
				vProcesses[Shortest].ArrivalTime
			);
		}

		CurrentTime++;
	}
}

void RoundRobin(vector<stProcess>& vProcesses, int Quantum, vector<stGanttSegment>& vGanttChart)
{
	int CurrentTime = 0;
	bool Done;

	for (stProcess& P : vProcesses)
	{
		P.RemainingTime = P.BurstTime;
	}

	do
	{
		Done = true;

		for (stProcess& P : vProcesses)
		{
			if (P.RemainingTime > 0)
			{
				Done = false;

				UpdateCurrentTime(CurrentTime, P.ArrivalTime);

				if (P.RemainingTime > Quantum)
				{
					vGanttChart.push_back({ P.Name, CurrentTime, CurrentTime + Quantum });

					CurrentTime += Quantum;
					P.RemainingTime -= Quantum;
				}
				else
				{
					vGanttChart.push_back({ P.Name, CurrentTime, CurrentTime + P.RemainingTime });

					CurrentTime += P.RemainingTime;
					P.FinishTime = CurrentTime;
					P.WaitingTime = CalculateWaitingTime((P.FinishTime - P.BurstTime), P.ArrivalTime);
					P.TurnaroundTime = CalculateTurnaroundTime(P.FinishTime, P.ArrivalTime);
					P.RemainingTime = 0;
				}
			}
		}
	} while (!Done);
}

double CalculateCpuUtilization(vector<stProcess>& vProcess)
{
	int totalBurstTime = 0, finishTime = 0;

	for (stProcess& P : vProcess)
	{
		totalBurstTime += P.BurstTime;

		if (P.FinishTime > finishTime)
		{
			finishTime = P.FinishTime;
		}
	}

	return  (double)totalBurstTime / finishTime * 100.0;
}

double CalculateAverageWaitingTime(vector<stProcess>& vProcess)
{
	double TotalWaitingTime = 0;

	for (stProcess& P : vProcess)
	{
		TotalWaitingTime += P.WaitingTime;
	}

	return TotalWaitingTime / vProcess.size();
}

double CalculateAverageeTurnaroundTime(vector<stProcess>& vProcess)
{
	double TotalTurnaroundTime = 0;

	for (stProcess& P : vProcess)
	{
		TotalTurnaroundTime += P.TurnaroundTime;
	}

	return TotalTurnaroundTime / vProcess.size();
}

void PrintResultsHeader(string AlgorithmName)
{
	cout << "\n\t  =======" << AlgorithmName << "======= \n";

	cout << "\n\t\t  ======= Result =======\n";
	cout << "+-----------------------------------------------------------+\n";
	cout << "| Process | Arrival | Burst | Finish | Waiting | Turnaround |\n";
	cout << "+-----------------------------------------------------------+\n";
}

void PrintValues(vector<stProcess>& vProcess)
{
	for (stProcess& P : vProcess)
	{
		cout << "| " << ""
			<< setw(4) << P.Name << "    | "
			<< setw(4) << P.ArrivalTime << "    | "
			<< setw(3) << P.BurstTime << "   | "
			<< setw(3) << P.FinishTime << "    | "
			<< setw(4) << P.WaitingTime << "    |"
			<< setw(6) << P.TurnaroundTime << "      |\n";
	}
	cout << "+-----------------------------------------------------------+\n";
}

void PrintGanttChart(vector<stGanttSegment>& vGanttChart)
{
	cout << "\nGantt Chart:\n";

	for (stGanttSegment& segment : vGanttChart)
	{
		cout << "+";
		cout << string(segment.EndTime - segment.StartTime, '-');
	}
	cout << "+\n";

	for (stGanttSegment& segment : vGanttChart)
	{
		cout << "|";
		int width = segment.EndTime - segment.StartTime;
		string name = segment.ProcessName;
		int padding = (width - name.length()) / 2;
		cout << string(padding, ' ') << name << string(width - padding - name.length(), ' ');
	}
	cout << "|\n";

	for (stGanttSegment& segment : vGanttChart)
	{
		cout << "+";
		cout << string(segment.EndTime - segment.StartTime, '-');
	}
	cout << "+\n";

	for (size_t i = 0; i < vGanttChart.size(); ++i)
	{
		const stGanttSegment& segment = vGanttChart[i];
		if (i == 0)
		{
			cout << segment.StartTime;
		}
		cout << string(segment.EndTime - segment.StartTime, ' ') << segment.EndTime;
	}
	cout << endl;
}

void DisplayResult(vector<stProcess>& vProcess, string AlgorithmName, vector<stGanttSegment>& vGanttChart)
{
	PrintResultsHeader(AlgorithmName);

	PrintValues(vProcess);

	PrintGanttChart(vGanttChart);

	cout << "\nAverage Waiting Time: {" << CalculateAverageWaitingTime(vProcess) << "}\n";

	cout << "\nAverage Turnaround Time: {" << CalculateAverageeTurnaroundTime(vProcess) << "}\n";

	cout << "\nCPU Utilization: {" << CalculateCpuUtilization(vProcess) << " %}\n";
}

void PrintFinishMessage(string Message)
{
	cout << Message;
}

void DisplayMainMenu()
{
	cout << "=====================================================\n";
	cout << "         Process Scheduling Algorithms\n";
	cout << "=====================================================\n";
	cout << "        [1] First Come First Served.\n";
	cout << "        [2] Shortest Job First 'Preemptive'.\n";
	cout << "        [3] Round Robin.\n";
	cout << "        [4] Exit.\n";
	cout << "======================================================\n";
}

void ResetValues(vector<stProcess>& vProcesses, vector<stGanttSegment>& vGanttChart)
{
	for (stProcess& P : vProcesses)
	{
		P.FinishTime = 0;
		P.WaitingTime = 0;
		P.TurnaroundTime = 0;
		P.RemainingTime = 0;
	}

	vGanttChart.clear();
}

void StartProgram(vector<stProcess> vProcesses, int Quantum, vector<stGanttSegment> vGanttChart)
{
	int choice = 0;

	do
	{
		system("cls");
		DisplayMainMenu();

		choice = ReadNumber("\nPlease Enter the Algorithm number you want to Use [1-4] : ");

		switch (choice)
		{
		case 1:

			system("cls");
			FirstComeFirstServed(vProcesses, vGanttChart);
			DisplayResult(vProcesses, "First Come First Served", vGanttChart);
			ResetValues(vProcesses, vGanttChart);
			cout << "\n\nPress any key to return to the Main Menu ...";
			system("pause>0");

			break;

		case 2:

			system("cls");
			ShortestJobFirstPreemptive(vProcesses, vGanttChart);
			DisplayResult(vProcesses, "Shortest Job First 'Preemptive'", vGanttChart);
			ResetValues(vProcesses, vGanttChart);
			cout << "\n\nPress any key to return to the Main Menu ...";
			system("pause>0");

			break;

		case 3:

			system("cls");
			RoundRobin(vProcesses, Quantum, vGanttChart);
			DisplayResult(vProcesses, "Round Robin", vGanttChart);
			cout << "\n\nPress any key to return to the Main Menu ...";
			ResetValues(vProcesses, vGanttChart);
			system("pause>0");

			break;

		case 4:

			PrintFinishMessage("Thank you for using our Program...We hope your experience is Useful ;)");

			break;

		default:

			cout << "You Will not reach here...";
			break;
		}

	} while (choice != 4);
}

int main()
{
	vector<stGanttSegment> vGanttChart;

	vector<stProcess> vProcesses = LoadProcessesDataFromFile(FileName);

	int Quantum = ReadQuantumFromFile(FileName);

	StartProgram(vProcesses, Quantum, vGanttChart);

	system("pause>0");
}
