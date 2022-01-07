// How to compile: gcc thread_with_multiple_join.c -o thread_with_multiple_join.o -lpthread
// How to run: ./thread_with_multiple_join.o
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
queue<string> jobQueue;
vector<string> words;
ofstream outFile;
pthread_mutex_t QueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;
std::set<pair<string, string>> results;
pthread_cond_t cond;


vector<string> tokenizer(string fileName){

	vector<string> tokens;

    ifstream inFile;
    inFile.open(fileName);

    //cout << fileName <<" ";

    string nextToken;
    while (inFile >> nextToken) {
    	//cout << nextToken <<" ";
    	tokens.push_back(nextToken);
    }

    inFile.close();

    return tokens;
    
}
	
vector<string> intersection_(vector<string> &v1, vector<string> &v2){
    vector<string> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_intersection(v1.begin(),v1.end(),
                          v2.begin(),v2.end(),
                          back_inserter(v3));

    return v3;
}

vector<string> union_(vector<string> &v1, vector<string> &v2){

    vector<string> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_union(v1.begin(),v1.end(),
                          v2.begin(),v2.end(),
                          back_inserter(v3));

    v3.erase( unique( v3.begin(),v3.end() ), v3.end() );

    return v3;
}

double J(vector<string> &v1, vector<string> &v2){
	
	vector<string> inter = intersection_(v1,v2);
	vector<string> uni = union_(v1, v2);

	return double(inter.size()) / double(uni.size());
}


vector<string> sentences(string fileName){

	ifstream input_file(fileName);
    
    string line = string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

    vector <string> sentences;
     

    stringstream check1(line);
     
    string intermediate;
     
    // Tokenizing w.r.t. space ' '
    while(getline(check1, intermediate, '.'))
    {
        sentences.push_back(intermediate);
    }

    input_file.close();

    return sentences;
     

}

void calculator(string fileName){

    vector<string> tokens = tokenizer(fileName);
    double j = J(words, tokens);
    stringstream stream;
    stream << fixed << std::setprecision(4) << j;
    string s = stream.str();
    pair<string,string> p = make_pair(s, fileName);

    pthread_mutex_lock(&resultMutex);
    results.insert(p);
    pthread_mutex_unlock(&resultMutex);

}

string theFunc(string fileName, int id){


    cout << "ok3" <<" " ;

    string str = "";

	str = str + "###" + "\n";
    pthread_mutex_lock(&resultMutex);
	result = result + 1;
    pthread_mutex_unlock(&resultMutex);
	str = str + "Result " + to_string(result) + ":" + "\n";
	str = str + "File: " + fileName + "\n";

	vector<string> tokens = tokenizer(fileName);

	double j = J(words, tokens);

    stringstream stream;
    stream << fixed << std::setprecision(4) << j;
    string s = stream.str();

	str = str + "Score: " + s + "\n";

	vector<string> sentences_ = sentences(fileName);

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
    				summary.push_back(sentence);
    				found = true;
    				break;
    			}
    		}
    		if(found){
    			break;
    		}
    	}
	}

	str = str + "Summary: ";

	for(int i = 0; i < summary.size(); i++){
		str = str + summary[i] + ".";
	}

    return str + "\n";

}

void* start(void* arg){

    if(jobQueue.size()==0){
        pthread_cond_signal(&cond);
        pthread_exit(0);
    }
    pthread_mutex_lock(&QueueMutex);
    
    string fileName = jobQueue.front();
    jobQueue.pop();

    pthread_mutex_unlock(&QueueMutex);

    auto myid = pthread_self();
    stringstream ss;
    ss << myid;
    string mystring = ss.str();

    pthread_mutex_lock(&fileMutex);

    outFile << "Thread " + mystring + " is calculating " + fileName + "\n";

    pthread_mutex_unlock(&fileMutex);

    calculator(fileName);

   /* pthread_mutex_unlock(&fileMutex);

    string result = theFunc(fileName, pthread_self());

    pthread_mutex_lock(&fileMutex);*/

    //outFile << result;

    //pthread_mutex_unlock(&fileMutex);

    start(NULL);

    pthread_exit(0);
}
int main(int argc,char* argv[]){

    pthread_cond_init(cond, NULL);


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
	
	int T = argArray[0];
	int A = argArray[1];
	int N =argArray[2];

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


    char c = 'A';


    //ofstream outFile;
    outFile.open(outputFileName);

    for(int i =0;i<T;i++){
        pthread_create(&threads[i], NULL, &start, NULL);
        pthread_setname_np(threads[i], &c);
        c = c + 1;
    }
    for(int i =0;i<T;i++){
        pthread_join(threads[i], NULL);
    }
    outFile.close();

   /* vector<string> sent =  sentences("abstract_1.txt");

    for(int i = 0 ; i < sent.size() ; i++){
    	cout << sent[i] << "####";
    }*/


	/*pthread_t thread1, thread2;
	if (pthread_create(&thread1, NULL, &myturn, NULL) != 0){
		printf("An error occured while creating the thread");
	}
	if (pthread_create(&thread2, NULL, &myturn, NULL) != 0){
		printf("An error occured while creating the thread");
	}
	if (pthread_join(thread1, NULL) != 0 ){
		printf("An error occured while joining the threads");
	}
	if (pthread_join(thread2, NULL) != 0 ){
		printf("An error occured while joining the threads");
	}
	yourturn();*/
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&resultMutex);
	pthread_exit(NULL);
	//return 0;
 }
