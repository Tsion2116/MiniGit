#include <iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<ctime>
#include<direct.h>
using namespace std;
//---------------------Data Structures----------------------





//---------------------Helper Functions---------------------



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

