#include <iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<ctime>
#include<direct.h>
#include<io.h>
using namespace std;
//---------------------Data Structures----------------------
struct Blob{
    string hash;
    string content;
};
struct Commit{
    string hash;
    vector<string> parents;
    string message;
    time_t timestamp;
    unordered_map<string,string>
    filehashes; 
};
//global state
unordered_map<string, string>
branches;
unordered_set<string> stagedFiles;
string currentBranch ="master";
Commit currentCommit;
//---------------------Helper Functions---------------------
bool dirExists(const string& path){
    return (_access(path.c_str(),0)==0);
}
bool fileExists(const string& filename){
    ifstream file(filename);
    return file.good();
}
string readFile(const string& filename){
    ifstream file(filename);
    return
    string((istreambuf_iterator<char>(file)),
    istreambuf_iterator<char>());
}
void writeFile(const string& filename,const string& content){
    ofstream file(filename);
    file<< content;
}
string computeHash(const string& content){
    unsigned long hash= 5381;
    for(char c : content) hash =((hash << 5) +hash) +c;
    stringstream ss;
    ss << hex<< hash;
    return ss.str();
}
//---------------------Main Functions---------------------
 void init(){
    if (dirExists(".minigit")){
        cout<<"MiniGit repository already exists.\n";
        return;
       }
       _mkdir(".minigit");
       _mkdir(".minigit/objects");
       _mkdir(".minigit/refs");
       // initial commit
       Commit initialCommit;
       initialComit.hash =computeHash("init");
       initialCommit.message = "Initial commit";
       initialCommit.timestamp = time(nullptr);     
         // Save initial commit 
         ofstream commitFile(".minigit/objects/" + initialCommit.hash);
            commitFile << initialCommit.message << "\n" << initialCommit.timestamp << "\n";
         commitFile.close();
         // Create HEAD
            ofstream headFile(".minigit/HEAD");
            headFile << "ref: refs/heads/master";
            headFile.close();
            cout << "Initialized empty MiniGit repository\n";
 }
void add(const string& filename) {
    if (!fileExists(filename)) {
        cerr << "Error: File '" << filename << "' does not exist.\n";
        return;
    }

    string content = readFile(filename);
    string hash = computeHash(content); 
    // Save blob in .minigit/objects
    ofstream blobFile(".minigit/objects/" + hash);
    blobFile << content;        
    blobFile.close();   
    stagedFiles.insert(filename);
    cout << "Added '" << filename << "' to staging area.\n";
}   

//---------------------CLI interface---------------------

