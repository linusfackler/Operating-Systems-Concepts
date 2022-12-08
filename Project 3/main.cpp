#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

struct Job {
    string name;
    int arrTime;
    int servTime;
    int done;
    int id;
};

void roundRobin(vector<Job> jobs);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Invalid number of arguments." << endl;
        cout << "Please use format:" << endl;
        cout << "g++ main.cpp -o main" << endl;
        cout << "main <inputfile> <Scheduling Algorithm>" << endl;
        return 0;
    }

    ifstream input;
    string filename = argv[1];
    input.open(filename);
    if (!input) {
        cout << "Could not open file. Check file name." << endl;
        cout << "Please use format:" << endl;
        cout << "g++ main.cpp -o main" << endl;
        cout << "main <inputfile> <Scheduling Algorithm>" << endl;
        return 0;
    }

    string n;
    int arr;
    int serv;
    vector<Job> jobs;
    int i = 0;
    while (input >> n >> arr >> serv)  {
        Job j;
        j.name = n;
        j.arrTime = arr;
        j.servTime = serv;
        j.done = 0;
        j.id = i;
        i++;

        jobs.push_back(j);
    }

    // for (int i = 0; i < jobs.size(); i++) {
    //     cout << jobs[i].name << "\t" << jobs[i].arrTime << "\t" << jobs[i].servTime << endl;
    // }

    string selectedJob = argv[2];

    if (selectedJob == "RR") {
        cout << "Round Robin" << endl << endl;
        roundRobin(jobs);
    }

    else if (selectedJob == "SRT") {
        cout << "srt";
    }

    else if (selectedJob == "FB") {
        cout << "feedback";
    }

    else if (selectedJob == "ALL") {
        cout << "all";
    }

    else {
        cout << "Unknown job!" << endl;
        cout << "Your options are: RR, SRT, FB, ALL" << endl << endl;
        cout << "RR: Round Robin (quantum = 1)" << endl;
        cout << "SRT: Shortest Remaining Time" << endl;
        cout << "FB: Multilevel Feedback (quantum = 1)" << endl;
        cout << "ALL: Executes all 3 scheduling algorithms" << endl << endl;
        cout << "Try again!" << endl;
        return 0;
    }


    return 0;
}

void roundRobin(vector<Job> jobs) {
    queue<Job> readyQueue;
    int numJobs = jobs.size();
    int finished = 0;
    int time = 0;
    Job current;
    current.done = 0;       // just so in first iteration it won't throw an error when checking if it's done before putting it back on the readyQueue
    int currentIndex;

    string top  = "  0    5    10   15   20";
    vector<string> outputs(numJobs);

    for (int i = 0; i < numJobs; i++) {
            outputs[i] = "";
    }

    for (Job i : jobs)
        if (i.arrTime == 0)
            readyQueue.push(i);

    while (finished < numJobs) {
        current = readyQueue.front();
        readyQueue.pop();
        currentIndex = current.id;

        

        for (int i = 0; i < numJobs; i++) {
            if (i == currentIndex)
                outputs[i] += "X";
            else
                outputs[i] += " ";
        }
        current.done++;
        time++;

        for (Job i : jobs)
            if (i.arrTime == time)
                readyQueue.push(i);

        if (current.done == current.servTime) {
            finished++;
        }

        else {
            readyQueue.push(current);
        }
    }

    cout << top << endl;
    for (int i = 0; i < numJobs; i++) {
        cout << jobs[i].name << " " << outputs[i] << endl;
    }
}

//void 