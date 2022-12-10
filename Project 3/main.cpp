#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;


// this struct is used to create a job
struct Job {
    string name;            // name of job (A, B, C, ...)
    int arrTime;            // arrival Time
    int servTime;           // service time
    int done;               // indicates how much it has been running already
    int id;                 // job id
    bool finished;          // indicates if job is finished (only used in SRT)
};

void roundRobin(vector<Job> jobs);
void SRT(vector<Job> jobs);
void FB(vector<Job> jobs);

int main(int argc, char *argv[]) {

    // if num of arguments is incorrect
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

    // if file couldn't open
    if (!input) {
        cout << "Could not open file. Check file name." << endl;
        cout << "Please use format:" << endl;
        cout << "g++ main.cpp -o main" << endl;
        cout << "main <inputfile> <Scheduling Algorithm>" << endl;
        return 0;
    }

    // temporary variables to set struct variables:
    string n;
    int arr;
    int serv;
    int i = 0;

    vector<Job> jobs;       // our main vector for all jobs
    
    while (input >> n >> arr >> serv)  {
        Job j;
        j.name = n;
        j.arrTime = arr;
        j.servTime = serv;
        j.done = 0;
        j.id = i;
        i++;
        j.finished = false;

        jobs.push_back(j);
    }

    // this is just used to print out the jobs:
    // for (int i = 0; i < jobs.size(); i++) {
    //     cout << jobs[i].name << "\t" << jobs[i].arrTime << "\t" << jobs[i].servTime << endl;
    // }

    string selectedJob = argv[2];       // selected job in command line

    if (selectedJob == "RR") {
        cout << "Round Robin (quantum = 1):" << endl << endl;
        roundRobin(jobs);
    }

    else if (selectedJob == "SRT") {
        cout << "Shortest Remaining Time:" << endl << endl;
        SRT(jobs);
    }

    else if (selectedJob == "FB") {
        cout << "Feedback (quantum = 1; 3 queues):" << endl << endl;
        FB(jobs);
    }

    else if (selectedJob == "ALL") {
        cout << "All 3 scheduling algorithms:" << endl << endl;

        cout << "Round Robin (quantum = 1):" << endl << endl;
        roundRobin(jobs);
        cout << endl;
        cout << endl;

        cout << "Shortest Remaining Time:" << endl << endl;
        SRT(jobs);
        cout << endl;
        cout << endl;

        cout << "Feedback (quantum = 1; 3 queues):" << endl << endl;
        FB(jobs);
        cout << endl;
    }

    // if job is not known
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
    queue<Job> readyQueue;          // our ready queue
    int numJobs = jobs.size();      // num of total jobs
    int finished = 0;               // num of finished jobs
    int time = 0;                   // current time in program
    Job current;                    // current job
    current.done = 0;       // just so in first iteration it won't throw an error when checking
                            // if it's done before putting it back on the readyQueue
    int currentIndex;       // indicates index of current job

    string top  = "  0    5    10   15   20";
    vector<string> outputs(numJobs);        // vector of strings, each job has own string

    for (int i = 0; i < numJobs; i++) {
            outputs[i] = "";
    }

    // first job detected
    for (Job i : jobs)
        if (i.arrTime == 0)
            readyQueue.push(i);
    
    // runs until all jobs finished
    while (finished < numJobs) {
        current = readyQueue.front();       // front of queue is in CPU
        readyQueue.pop();
        currentIndex = current.id;

        

        for (int i = 0; i < numJobs; i++) {
            if (i == currentIndex)          // for current job, adds X to string
                outputs[i] += "X";
            else
                outputs[i] += " ";
        }
        current.done++;
        time++;

        // new job arrived
        for (Job i : jobs)
            if (i.arrTime == time)
                readyQueue.push(i);

        // if current job is done
        if (current.done == current.servTime) {
            finished++;
        }

        // if current job is note done, push back on queue
        else {
            readyQueue.push(current);
        }
    }

    // print jobs
    cout << top << endl;
    for (int i = 0; i < numJobs; i++) {
        cout << jobs[i].name << " " << outputs[i] << endl;
    }
}

void SRT(vector<Job> jobs) {
    int numJobs = jobs.size();          // total num of jobs
    int finished = 0;                   // num of finished jobs
    int time = 0;                       // current time in program
    int currentIndex;                   // index of current job in cpu

    string top  = "  0    5    10   15   20";
    vector<string> outputs(numJobs);    // vector of strings, each job has own string

    for (int i = 0; i < numJobs; i++) {
            outputs[i] = "";
    }

    // first job detected
    for (Job i : jobs)
        if (i.arrTime == 0)
            currentIndex = i.id;

    while (finished < numJobs) {

        // adds X for current Job
        for (int i = 0; i < numJobs; i++) {
            if (i == currentIndex)
                outputs[i] += "X";
            else
                outputs[i] += " ";
        }

        jobs[currentIndex].done++;
        time++;

        // if current job done
        if (jobs[currentIndex].done == jobs[currentIndex].servTime) {
            finished++;
            jobs[currentIndex].finished = true;
            // indicate it's done, so it won't be chosen anymore
        }

        int min = INT_MAX;          // min set to highest int to compare
        string minName = "Z";       // min name set to highest char to compare
        // minName is used in the case that 2 jobs have the same time left
        // -> job with lower name gets chosen

        int timeLeft;

        vector<Job> availableJobs;  // vector for currently available jobs

        // new job arrived
        for (Job i : jobs) {
            if (!i.finished && i.arrTime <= time) {
                availableJobs.push_back(i);
            }
        }


        // check available jobs for next shortest
        for (Job i : availableJobs) {
            timeLeft = i.servTime - i.done;     // indicates time left for job
            if (timeLeft <= min) {
                if (timeLeft == min) {          // in case 2 jobs have same time left
                    if (i.name < minName) {
                        currentIndex = i.id;
                        min = timeLeft;
                        minName = i.name;
                    }
                }
                else {
                    currentIndex = i.id;        // set new current job, in case no other min found
                    min = timeLeft;             // set new min
                    minName = i.name;
                }
            }     
        }  
    }

    // print all jobs
    cout << top << endl;
    for (int i = 0; i < numJobs; i++) {
        cout << jobs[i].name << " " << outputs[i] << endl;
    }
}

void FB(vector<Job> jobs) {
    queue<Job> queue1;
    queue<Job> queue2;
    queue<Job> queue3;
    // -> 3 queues

    int numJobs = jobs.size();          // total num jobs
    int finished = 0;                   // num of finished jobs
    int time = 0;                       // current time in program
    int currentIndex;                   // index of current job
    int previousIndex;                  // index of previous job

    string top  = "  0    5    10   15   20";
    vector<string> outputs(numJobs);    // vector of strings, each job has own string

    for (int i = 0; i < numJobs; i++) {
            outputs[i] = "";
    }

    // find first job
    for (Job i : jobs)
        if (i.arrTime == 0)
            queue1.push(i);
    
    while (finished < numJobs) {

        // since Feedback chooses next job by queue priority, we will check from 1-3
        if (!queue1.empty()) {
            currentIndex = queue1.front().id;
            queue1.pop();

            // print X for current job
            for (int i = 0; i < numJobs; i++) {
                if (i == currentIndex)
                    outputs[i] += "X";
                else
                    outputs[i] += " ";
            }

            jobs[currentIndex].done++;
            time++;

            // new job arrived
            for (Job i : jobs) {
                if (i.arrTime == time) {
                    queue1.push(i);
                }
            }

            // if job is not finished
            if (jobs[currentIndex].servTime != jobs[currentIndex].done) {
                // if all other queues are empty, put in same queue
                if (queue1.empty() && queue2.empty() && queue3.empty()) {
                        queue1.push(jobs[currentIndex]);
                }

                // otherwise, put in succeeding queue
                else {
                    queue2.push(jobs[currentIndex]);
                    previousIndex = currentIndex;
                }
            }

            // if job finished
            else {
                finished++;
            }
        }

        else if (!queue2.empty()) {

            // if previous job same as current
            if (queue2.front().id == previousIndex) {
                if (queue3.empty()) {          // if next queue empty, choose this job
                    currentIndex = queue2.front().id;
                    queue2.pop();
                }

                else {                      // if next queue not empty, choose job from next queue
                    currentIndex = queue3.front().id;
                    queue3.pop();
                }
            }

            else {
                currentIndex = queue2.front().id;
                queue2.pop();
            }

            // print X for current job
            for (int i = 0; i < numJobs; i++) {
                if (i == currentIndex)
                    outputs[i] += "X";
                else
                    outputs[i] += " ";
            }

            jobs[currentIndex].done++;
            time++;

            // new job arrived
            for (Job i : jobs) {
                if (i.arrTime == time) {
                    queue1.push(i);
                }
            }

            if (jobs[currentIndex].servTime != jobs[currentIndex].done) {
                // if all other queues are empty, put in same queue
                if (queue1.empty() && queue2.empty() && queue3.empty()) {
                    queue2.push(jobs[currentIndex]);
                }
                else {
                    queue3.push(jobs[currentIndex]);
                    previousIndex = currentIndex;
                }
            }

            // if job finished
            else {
                finished++;
            }
        }

        else {      // in queue 3
            currentIndex = queue3.front().id;
            queue3.pop();

            // print X for current job
            for (int i = 0; i < numJobs; i++) {
                if (i == currentIndex)
                    outputs[i] += "X";
                else
                    outputs[i] += " ";
            }

            jobs[currentIndex].done++;
            time++;

            // new job arrived
            for (Job i : jobs)
                if (i.arrTime == time)
                    queue1.push(i);

            // if job not finished
            if (jobs[currentIndex].servTime != jobs[currentIndex].done) {
                queue3.push(jobs[currentIndex]);
                previousIndex = currentIndex;
            }

            // if job finished
            else {
                finished++;
            }
        }
    }

    // print all jobs
    cout << top << endl;
    for (int i = 0; i < numJobs; i++) {
        cout << jobs[i].name << " " << outputs[i] << endl;
    }
}