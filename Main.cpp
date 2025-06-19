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

void commit(const std::string& message) {
  if(stagedFiles.empty()) {
    std::cerr<< "no changes stages for commit.\n";
    return;
  }

  Commit newCommit;
  newCommit.parents.push_back(currentCommit.hash);
  newCommit.message = message;
  newCommit.timestamp = time(nullptr);
  newCommit.fileHashes = currentCommit.fileHashes;

    //update with staged files

  for(const auto& filename : stagedFiles) {
    std::string content = readFile(filename);
    newCommit.fileHashes[filename] = computeHash(content);
  }
  //compute commit hash

  std::stringstream commitData;
  commitData<< newCommit.parents[0]<<newCommit.message<<newCommit.timestamp;
  for(const auto& file : newCommit.fileHashes){
    commitData << file.first <<file.second;
  }
  newCommit.hash = computeHash(commitData.str());

  //save commit 

  std::ofstream commitFile(".minigit/objects/" + newCommit.hash);
  commitFile << newCommit.message <<"\n";
  commitFile << newCommit.timestamp <<"\n";
  
  for(const auto& parent : newCommit.parents) commitFile << parent << "";
  commitFile << "\n";
  for(const auto& file : newCommit.fileHashes){
    commitFile << file.first << "" <<file.second << "\n";
  }
  commitFile.close();

  //update references

  branches[currentBranch] = newCommit.hash;
  currentCommit = newCommit;
  stagedFiles.clear();

  std::cout << "" << currentBranch << "" << newCommit.hash.substr(0,7) << " " << message << "\n";
  
}

void log() {
    Commit commit = currentCommit;
    while(true){
        std::cout << "commit" << commit.hash << "\n";
        std::cout << "date:" <<std::ctime(&commit.timestamp);
        std::cout << "  " << commit.message << "\n\n";

    if(commit.parents.empty()) break;

    // load parent commit (simplified - in real impl you'd read from file)
    Commit parent;
    parent.Hash = commit.parents[0];
    commit = parent;
    }
}

//---------------------CLI interface---------------------

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "MiniGit - A simple version control system\n";
        return 1;
    }
    string command = argv[1];

    if (command == "init") {
        init();
    }
    else if (command == "add") {
        if (argc < 3) {
            cerr << "Error: Missing filename\n";
            return 1;
        }
        add(argv[2]);
    }
    else if (command == "commit") {
        if (argc < 4 || string(argv[2]) != "-m"){
            cerr << "rror: Use 'commit -m \"message\"'\n";
            return 1;
        }
        commit(argv[3]);
    }
    else if (command == "log") {
        if (argc < 3) {
            cerr << "Error: Missing branch name\n";
            return 1;   
        }
        cout << "Created branch '" << argv[2] << "'\n";
    }
    else if (command == "checkout") {
        if (argc < 3) {
            cerr << "Error: Missing branch name\n";
            return 1;
        }
        cout << "switched to branch '" << argv[2] << "'\n";
    }
    else if (command == "merge") {
        if (argc < 3) {
            cerr << "Error: Missing branch name\n";
            return 1;
        }
        cout << "merged '" << argv[2] << "'into current branch\n";
    }
    else {
        cerr << "Unknown command: " << command << "\n";
        return 1;
    }
    return 0;
}