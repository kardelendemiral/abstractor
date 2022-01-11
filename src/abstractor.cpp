/* @author Kardelen Demiral
    Main idea of the project is to process given input files simultaneously searching for keywords given.
    The program also summarizes these text files according to our input and calculates a score of similarity for each file. 
    Thren the most similar ones are printed to the output file.
    To process the files simultaneously, we use multithread programming. 

    Conclusion: This project gave me an insight of what multithread programming is and how it can be used in order to make our programs more efficient.
*/
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <utility>
#include <set>
using namespace std;

int result = 0;
queue<string> jobQueue; //jobs are in this queue for the threads to pop them
vector<string> words;
ofstream outFile;
pthread_mutex_t QueueMutex = PTHREAD_MUTEX_INITIALIZER; //queue mutex
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER; //file write mutex
pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER; //a mutex for the main thread to wait for the others to write the result into the file
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER; //result value mutex for it to be change mutual exclusively
std::multiset<pair<string, string>> results; //calculated results are stored in this set
pthread_cond_t cond; //a condition for the main thread to wait for the others


vector<string> tokenizer(string fileName){ //tokenizes words in a file

	vector<string> tokens;

    ifstream inFile;
    inFile.open(fileName);


    string nextToken;
    while (inFile >> nextToken) {
    	tokens.push_back(nextToken);
    }

    inFile.close();

    return tokens;
    
}
	
vector<string> intersection_(vector<string> &v1, vector<string> &v2){ //gives intersection of the two word vectors
    vector<string> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_intersection(v1.begin(),v1.end(), v2.begin(),v2.end(), back_inserter(v3));

    return v3;
}

vector<string> union_(vector<string> &v1, vector<string> &v2){ //gives the union of the two word vectors

    vector<string> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_union(v1.begin(),v1.end(), v2.begin(),v2.end(), back_inserter(v3));

    v3.erase( unique( v3.begin(),v3.end() ), v3.end() );

    return v3;
}

double J(vector<string> &v1, vector<string> &v2){ //calculates the Jaccard Similarity score for given two word vectors
	
	vector<string> inter = intersection_(v1,v2);
	vector<string> uni = union_(v1, v2);

	return double(inter.size()) / double(uni.size());
}


vector<string> sentences(string fileName){ //splits sentences in a file and returns a vector of sentences

	ifstream input_file(fileName);

    vector <string> sentences;
    string s = "";

    string word;
    while (input_file) {
        input_file >> word;
        if(word[0]=='\0' || word[0]=='\n' || word[0]=='\r'){
            word = word.substr(1);
        }
        s = s + " " + word;
    }

    stringstream check1(s);
     
    string intermediate;
    
    while(getline(check1, intermediate, '.'))
    {
        sentences.push_back(intermediate);
    }

    input_file.close();

    return sentences;
     

}

void calculator(string fileName){ //calculates the similarity between the words that are being searched and the words in the given file

    vector<string> tokens = tokenizer("../abstracts/" + fileName);
    double j = J(words, tokens);
    stringstream stream;
    stream << fixed << std::setprecision(4) << j;
    string s = stream.str();
    pair<string,string> p = make_pair(s, fileName);

    pthread_mutex_lock(&resultMutex);

    results.insert(p);
    pthread_mutex_unlock(&resultMutex);

}


void* start(void* id){ //starts the threads

    char *ptr = (char *)id;

    if(jobQueue.size()==0){ //if the queue is empty, terminate
        pthread_cond_signal(&cond);
        pthread_exit(0);
    }
    pthread_mutex_lock(&QueueMutex);
    
    string fileName = jobQueue.front(); //pop from the job queue
    jobQueue.pop();

    pthread_mutex_unlock(&QueueMutex);


    string s(1, *ptr);

    outFile << "Thread " + s + " is calculating " + fileName + "\n";

    pthread_mutex_unlock(&fileMutex);

    calculator(fileName);


    start(id); //when a thread finishes, it restarts to process a new file 

    pthread_exit(0);
}
int main(int argc,char* argv[]){

    pthread_cond_init(&cond, NULL);


	string inputFileName = argv[1];
	string outputFileName = argv[2];


	ifstream inputFile;
	inputFile.open(inputFileName);
	string arguments;
	string line;
	getline(inputFile, line);

	vector<int> argArray;
	stringstream ss(line); 
    string temp_str;


    while(getline(ss, temp_str, ' ')){ 
      argArray.push_back(stoi(temp_str));
    }
	
	int T = argArray[0]; //number of threads
	int A = argArray[1]; //number of abstract files to be processed
	int N =argArray[2]; //number of summaries to be printed

	getline(inputFile, line);
	stringstream ss2(line);

    while(getline(ss2, temp_str, ' ')){ 
      words.push_back(temp_str);
    }


    for(int i = 0; i < A ;i++){
    	getline(inputFile, line);
    	jobQueue.push(line);
    }

    inputFile.close();

    vector<pthread_t> threads(T);


    char c =  'A';

    char tids[T];
    for(int i =0;i<T;i++){ //give names to threads
        tids[i] = c;
        c = c + 1;
    }
    //ofstream outFile;
    outFile.open(outputFileName);

    for(int i =0;i<T;i++){
        pthread_create(&threads[i], NULL, &start, &tids[i]); //create the threads
    }
    for(int i =0;i<T;i++){
        pthread_join(threads[i], NULL); //join them
    }

    pthread_mutex_lock(&waitMutex); 

    if(jobQueue.size() != 0){  //main thread waits for the others to finish execution
        pthread_cond_wait(&cond, &waitMutex);
    } 

    set<pair<string, string>>::reverse_iterator itr;

    int count = 0;

    outFile << "###" << "\n";

    for (itr = results.rbegin(); itr != results.rend(); itr++) { //iterate the results set in reverse order to print results
        
        if(count == N){ //N summaries are enough
            break;
        }
        string score = (*itr).first; 
        string name = (*itr).second;


        result = result + 1;
        outFile << "Result " << to_string(result) << ":" << "\n";
        outFile << "File: " << name << "\n";

        outFile << "Score: " << score << "\n";

        vector<string> sentences_ = sentences("../abstracts/" + name);

        vector<string> summary;

        for(int i = 0; i < sentences_.size() ; i++){ 
            string sentence = sentences_[i];
            vector<string> wordsInSentence;
            stringstream check1(sentence);
     
            string intermediate;
            while(getline(check1, intermediate, ' ')){
                wordsInSentence.push_back(intermediate);
            }
            bool found = false;

            for(int j = 0; j < words.size(); j++){
                for(int k = 0; k < wordsInSentence.size(); k++){
                    if(words[j] == wordsInSentence[k]){
                        
                        summary.push_back(sentence); //for every sentence in a given file, if there is at least one common word with the query, add it to the summary.
                        found = true;
                        break;
                    }
                }
                if(found){
                    break;
                }
            }
        }

        outFile << "Summary:";

        for(int i = 0; i < summary.size(); i++){ //write the summary
            outFile << summary[i] <<".";
        }

        count++;
        outFile << " \n"<< "###" << "\n";

    }

    pthread_mutex_unlock(&waitMutex);


    outFile.close();
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&resultMutex);
    pthread_mutex_destroy(&waitMutex);
    pthread_mutex_destroy(&QueueMutex);
	pthread_exit(NULL);
	//return 0;
 }
