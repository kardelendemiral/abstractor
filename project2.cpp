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
#include <mutex>
#include <fstream>
using namespace std;

int result = 0;


void* myturn( void *arg){
	int i;
	for(i=0; i < 2; i++){
		sleep(2);
		printf("If you're happy and you know it clap your hands! - %d\n", i);
		sleep(2);
		printf("CLAP CLAP! - %d\n", i);
	}
	return NULL;
}
void yourturn(){
	int i;
  	for(i=0; i < 2; i++){
		printf("Thanks! - %d\n", i);
		sleep(2);
	}
}



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

void theFunc(ofstream &outFile, string abstractFileName, vector<string> &words){

	outFile << "Thread ... is calculating " << abstractFileName <<"\n"; //bu mainde de yapılabilir


	// MUTEX

	outFile << "###" << "\n";
	result = result + 1;
	outFile << "Result " << result << ":" << "\n";
	outFile << "File: " << abstractFileName << "\n";

	vector<string> tokens = tokenizer(abstractFileName);

	double j = J(words, tokens);

	outFile << "Score: " << setprecision(4) << fixed << j << "\n";

	vector<string> sentences_ = sentences(abstractFileName);

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

	outFile << "Summary: ";

	for(int i = 0; i < summary.size(); i++){
		outFile << summary[i] << ".";
	}

}

int main(int argc,char* argv[]){

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
	vector<string> words;
	stringstream ss2(line);

    while(getline(ss2, temp_str, ' ')){ 
      words.push_back(temp_str);
    }

    vector<string> abstracts;

    for(int i = 0; i < A ;i++){
    	getline(inputFile, line);
    	abstracts.push_back(line);
    }

    inputFile.close();

    vector<pthread_t> threads;

    for(int i = 0; i < T; i++){
    	//create threads
    }

    ofstream outFile;
    outFile.open(outputFileName);

    theFunc(outFile, abstracts[0], words);

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
	pthread_exit(NULL);
	//return 0;
 }
