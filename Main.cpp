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
    unordered_map<string,string>fileHashes; 
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
       initialCommit.hash =computeHash("init");
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
    if(commit.hash.empty()){
        cerr<<"Error: no commits to display.\n";
        return;
    }
    while(!commit.hash.empty()){
        cout << "commit" << commit.hash << "\n";
        cout << "Date:" << ctime(&commit.timestamp);
        cout << "  " << commit.message << "\n\n";

    if(commit.parents.empty()) break;
    commit = loadCommit(commit.parents[0]);
    }
}
void createBranch(const string& branchName){
    if(branches.find(branchName)!=branches.end()){
        cerr<< "Error: Branch '" << branchName << "' already exists.\n";
        return;
    }
    branches[branchName] = currentCommit.hash;

//create branch file
    ofstream branchFile(".minigit/refs/heads/" + branchName);
    if(!branchFile){
        cerr<<"Error: Failed to create branch file.\n";
        return;
    }
    branchFile << currentCommit.hash;
    branchFile.close();

    cout << "Created branch '"<< branchName << "'\n";
}

void checkoutBranch(const string& branchName){
    if(branches.find(branchName)==branches.end()){
        cerr <<"Error: Branch '" << branchName << "' does not exist.\n";
        return;
    }

    string commitHash = branches[branchName];
    Commit targetCommit = loadCommit(commitHash);
    updateWorkingDirectory(targetCommit);

    //update HEAD
    ofstream headFile(".minigit/HEAD");
    if(!headFile){
        cerr<<"Error: Failed to update HEAD.\n";
        return;
    }
    headFile << "ref: refs/heads/" << branchName;
    headFile.close();

    currentBranch= branchName;
    currentCommit= targetCommit;
    cout<<"Switched to branch '"<<branchName<< "'\n";
}

string findLCA(const string& commitHash1, const string& commitHash2){
    map<string,bool> visited;
    vector<string> toVisit1={commitHash1};
    vector<string> toVisit2={commitHash2};

    while(!toVisit1.empty() || !toVisit2.empty()){
        if(!toVisit1.empty()){
            string current=toVisit1.back();
            toVisit1.pop_back();
            if(visited[current]) return current;
            visited[current]=true;

            Commit commit=loadCommit(current);
            for(const string& parent: commit.parents){
                toVisit1.push_back(parent);
            }
        }
        if(!toVisit2.empty()){
            string current=toVisit2.back();
            toVisit2.pop_back();
            if(visited[current]) return current;
            visited[current]=true;

            Commit commit=loadCommit(current);
            for(const string& parent:commit.parents){
                toVisit2.push_back(parent);
            }
        }
    }
    return ""; // no common ancestor found 
}

void merge(const string& branchName){
    if(branches.find(branchName) == branches.end()){
        cerr << "Error: Branch '"<<branchName<< "' does not exist.\n";
        return;
    }
    if(currentBranch == branchName){
        cerr<<"Error: Cannot merge a branch with itself.\n";
        return;
    }

    string ourCommitHash=currentCommit.hash;
    string theirCommitHash=branches[branchName];

    // Find LCA
    string lcaHash = findLCA(ourCommitHash, theirCommitHash);
    if(lcaHash.empty()){
        cerr<<"Error: Could not find common ancestor\n";
        return;
    }

    Commit lcaCommit=loadCommit(lcaHash);
    Commit ourCommit=currentCommit;
    Commit theirCommit=loadCommit(theirCommitHash);

    bool hasConflicts=false;
    unordered_set<string> allFiles;

    // Collect all files from all three commits 
    for(const auto& file:lcaCommit.fileHashes){
        allFiles.insert(file.first);
    }
    for(const auto& file:ourCommit.fileHashes){
        allFiles.insert(file.first);
    }
    for(const auto& file:theirCommit.fileHashes){
        allFiles.insert(file.first);
    }

    // perform three-way merge for each file 
    for(const string& filename:allFiles){
        string lcaHash=lcaCommit.fileHashes.count(filename)? lcaCommit.fileHashes.at(filename):"";
        string ourHash=ourCommit.fileHashes.count(filename)? ourCommit.fileHashes.at(filename):"";
        string theirHash=theirCommit.fileHashes.count(filename)? theirCommit.fileHashes.at(filename):"";

        if(ourHash==theirHash){
            // no changes or both changed the same way
            continue;
        } else if(lcaHash==ourHash){
            // we didn't change it, take their version 
            if(!theirHash.empty()){
                ifstream blobFile(".minigit/objects/"+ theirHash);
                string content((istreambuf_iterator<char>(blobFile)), istreambuf_iterator<char>());
                writeFile(filename, content);
                stagedFiles[filename]=theirHash;
            }
        } else if (lcaHash==theirHash){
            // they didn't change it, keep our version 
            continue;
        }else{
            // conflict - both modified differently 
            cerr<<"CONFLICT: Both modified" << filename <<"\n";
            hasConflicts=true;

            // create conflict markers 
            string ourContent, theirContent;
            if(!ourHash.empty()){
                ifstream ourBlob(".minigit/objects/"+ ourHash);
                ourContent= string((istreambuf_iterator<char>(ourBlob)), istreambuf_iterator<char>());
            }
             if(!theirHash.empty()){
                ifstream theirBlob(".minigit/objects/"+ theirHash);
                theirContent= string((istreambuf_iterator<char>(theirBlob)), istreambuf_iterator<char>());
            }

            string conflictContent="<<<<<<< HEAD\n"+ ourContent + "=======\n" + theirContent + ">>>>>>> "+ branchName+"\n";
            writeFile(filename, conflictContent);
            stagedFiles[filename]= computeHash(conflictContent);
        }
    }
    saveStagedFiles();

    if(hasConflicts){
        cout<<"Automatic merge failed; fix conflicts and then commit result.\n";
    } else{
        // create merge commit
        string message="Merge branch '"+branchName+"' into "+currentBranch;
        commit(message);
    }
}

void diff(const string& commitHash1, const string& commitHash2){
    Commit commit1=loadCommit(commitHash1);
    Commit commit2=loadCommit(commitHash2);

    if(commit1.hash.empty() || commit2.hash.empty()){
        cerr << "Error: Invalid commit hashes\n";
        return;
    }

    unordered_set<string> allFiles;
    for(const auto& file: commit1.fileHashes) allFiles.insert(file.first);
    for(const auto& file: commit2.fileHashes) allFiles.insert(file.first);

    for(const string& filename:allFiles){
        string hash1=commit1.fileHashes.count(filename)? commit1.fileHashes.at(filename):"";
        string hash2=commit2.fileHashes.count(filename)? commit2.fileHashes.at(filename):"";

        if(hash1==hash2) continue;
        cout<<"diff --git a/" <<filename<<" b/" <<filename<<"\n";

        string content1, content2;
        if(!hash1.empty()){
            ifstream blob1(".minigit/objects/" + hash1);
            content1= string((istreambuf_iterator<char>(blob1)), istreambuf_iterator<char>());
        }
        if(!hash2.empty()){
            ifstream blob2(".minigit/objects/"+ hash2);
            content2= string((istreambuf_iterator<char>(blob2)), istreambuf_iterator<char>());
        }

        // simple line-by-line diff
        istringstream stream1(content1);
        istringstream stream2(content2);
        string line1, line2;
        int lineNum=1;

        while(getline(stream1, line1) || getline(stream2, line2)){
            if(line1!=line2){
                cout<<"@"<<lineNum<< " +"<< lineNum<<" @"<<"\n";
                if(!line1.empty()) cout<<"-"<<line1<<"\n";
                if(!line2.empty()) cout<<"+"<< line2<<"\n";
            }
            lineNum++;
            line1.clear();
            line2.clear();
        }
    }
}
//---------------------CLI interface---------------------

int main(int argc, char* argv[]) {
    if(dirExists(".minigit")){
        loadBranches(); // add this line
        loadStagedFiles(); // load staged files at startup 
        currentCommit = loadCurrentCommit();
    }
    if (argc < 2) {
        cout << "MiniGit - A simple version control system\n";
        cout << "usage: minigit <command> [<args>]\n";
        cout << "commands:\n";
        cout << " init\n";
        cout << "add <file>\n";
        cout << " commit -m \"message\"\n";
        cout << "log\n";
        cout << "branch <branch-name>\n";
        cout << "checkout <branch-name>\n";
        cout << "merge <branch-name>\n";
        cout << "diff <commit1> <commit2>\n";
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
            cerr << "Error: Use 'commit -m \"message\"'.\n";
            return 1;
        }
        commit(argv[3]);
    }
    else if (command == "log") {
        log();
    }
    else if (command == "branch"){
        if (argc == 2) {
            cout << "available branches:\n";
            for(const auto& [name,hash] : branches){
                cout << (name == currentBranch ? "*" : " ") << name << "\n";
            }
        }
        else if(arg == 3){
            createBranch(argv[2]);
        }else {
            cerr<< "usage:minigit branch [<branch-name>]\n";
            return 1;
        }
    }
    else if (command == "checkout") {
        if (argc < 3) {
            cerr << "Error: Missing branch name or commit hash.\n";
            return 1;
        }
        checkoutBranch(argv[2]);
    }
    else if (command == "merge") {
        if (argc < 3) {
            cerr << "Error: Missing branch name.\n";
            return 1;
        }
        merge(argv[2]);
    }
    else if(command == "diff"){
        if(argc < 4){
            cerr<< "Error: need two commit hashes.\n";
            return 1;
        }
        diff(argv[2],argv[3]);
    }
    else {
        cerr << "Error: Unknown command: " << command << "\n";
        return 1;
    }
    return 0;
}