#ifdef WINDOWS
	#include "stdafx.h"
#endif
#include "Command.h"
using namespace std;

vector<string> arg_vec;
deque<string> commandList;
string command;
map<string, string> env;
extern char * hPath;
extern pid_t recentPid;
extern pid_t waitingPid;
extern int status;
extern bool hasAmp;
extern bool xFlag;
char * dPath;
bool isExit;

bool isWord(string s){
	int count = 0;
	for (char c : s){
		if (isalpha(c)){
			count++;
		}
	}
	if (count == 0){
		return false;
	}
	else {
		return true;
	}
}

bool built_in_command(string s){
	if (s == "show" || s == "set" || s == "unset" || s == "export" || s == "unexport" || s == "environ" || s == "chdir" || s == "environ" || s == "wait" || s == "exit" || s == "clr" || s == "dir" || s == "echo" || s == "help" || s == "pause" || s == "history" || s == "repeat" || s == "kill"){
		return true;
	}
	else {
		return false;
	}
}

void addCommand(string s){
	if (commandList.size() >= 100){
		commandList.pop_front();
		commandList.push_back(s);
	}
	else{
		commandList.push_back(s);
	}
}

void parse_command(string c){

	istringstream iss(c);
	string result;
    isExit = false;
	while (iss >> result){
        if (result == "$$"){
            std::string intToString = std::to_string(getpid());
            result = intToString;
            if (xFlag)
                cout << "$$ is " << result << endl;
        } else if (result == "$?"){
            std::string intToString = std::to_string(status);
            result = intToString;
            if (xFlag)
                cout << "$? is " << result << endl;
        } else if (result == "$!"){
            std::string intToString = std::to_string(waitingPid);
            result = intToString;
            if (xFlag)
                cout << "$! is " << result << endl;
        }
		arg_vec.push_back(result);
	}
}


int exe(vector<string> arg_vec){
	// helper method to add the command to the command history list
	// keeps track of the last 100 commands.
	addCommand(command);

	string com = arg_vec[0];

	//kill
	if (com == "kill"){
		if (arg_vec.size() != 3){
			cout << "Command Error" << endl;
			return Done;
		}
		else {
			string com2 = arg_vec[1];
			string com3 = arg_vec[2];
			char* com2Char = const_cast<char*>(com2.c_str());
			if (*com2Char != '-'){
				cout << "Command Error" << endl;
				return Done;
			}
			else {
				int sid;
				int pd;
				com2Char++;
				istringstream iss(com2Char);
				iss >> sid;
				istringstream iss2(com3);
				iss2 >> pd;
				kill(pd, sid);
			}
		}
	}

	// wait
	if (com == "wait"){
		if (arg_vec.size() != 2){
			perror("Command Error");
			return Done;
		}
		else {
			int iD;
			string com2 = arg_vec[1];
			istringstream iss(com2);
			iss >> iD;
			pid_t pid = iD;
			waitpid(pid, NULL, 0);

			/*else {
				cout << "Command Error" << endl;
				return Done;
			}*/
		}
	}

	// pause
	if (com == "pause"){
        if (arg_vec.size() == 1){
            return Pause;
        }
        else {
            perror("Command Error");
            return Done;
        }
	}

    // help
    if (com == "help"){

        pid_t pid;
        if (pid < 0)
            perror("Fork Error");
        else if ((recentPid=pid = fork()) == 0) {
            fflush(stdout);
            execlp("vi", "vi", "man/XSSHman",0);
            perror("execvp error");
            exit(1);
        } else{
            if (!hasAmp){
                waitpid(pid, &status, 0);
            }
            else waitingPid = pid;
        }

        return Done;
    }

    if (com == "dir"){
        DIR *dir;
        struct dirent *ent;
        //string trial = string(dPath);
        //string prefix = "C:/cygwin64";
        char* pathh = getenv("PWD");
        //const char* pathh = prefix.c_str();
        if ((dir = opendir(pathh)) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir(dir)) != NULL) {
                printf("%s\n", ent->d_name);
            }
            closedir(dir);
        }
        else {
            /* could not open directory */
            perror("");
            return EXIT_FAILURE;
        }
    }

    // chdir
    if (com == "chdir"){
        //cout << "path: " << getenv("PWD") << endl;
        char * pathIs = getenv("PWD");
        //cout << "pathIS: " << pathIs << endl;
        if (arg_vec.size() == 1){
            //cout << "here" << endl;
            string putPath = "PWD=";
            string s(hPath);
            putPath += s;
            //cout <<"putPath: " << putPath << endl;
            setenv("PWD", s.c_str(), 1);
            //putenv(const_cast<char*>(putPath.c_str()));
        }
        else if (arg_vec.size() == 2){
            string::const_iterator it = arg_vec[1].begin();
            if (arg_vec[1] == "."){
                return Done;
            }
                // fix this
            else if (arg_vec[1] == "..") {
                string cutPath(pathIs);
                if (cutPath.length() == 1){
                    return Done;
                }
                //st = string(dPath).substr(0, string(dPath).size() - 1);
                int count = 0;
                for (std::string::reverse_iterator rIter = cutPath.rbegin(); rIter != cutPath.rend(); ++rIter){
                    if (count == 0){
                        cutPath.erase(--(rIter.base()));
                        count++;
                        continue;
                    }
                    if (*rIter != '/'){
                        cutPath.erase(--(rIter.base()));
                    }
                    else {
                        break;
                        //cutPath.erase(--(rIter.base()));
                        //break;
                    }
                }
                string stringIs = "PWD=";
                stringIs += cutPath;
                //cout << "stringIs: " << stringIs << endl;
                setenv("PWD", cutPath.c_str(), 1);
                //putenv(const_cast<char*>(stringIs.c_str()));
            }
            else if (*it == '/'){
                string stringIs = "PWD=";
                stringIs += arg_vec[1];
                setenv("PWD", arg_vec[1].c_str(), 1);
                //putenv(const_cast<char*>(stringIs.c_str()));
            }
            else {
                /*if (pathIs == NULL){
                    return Done;
                }*/
                string changeString(pathIs);
                string stringIs = "PWD=";
                string::reverse_iterator rI = changeString.rbegin();
                if (*rI != '/'){
                    changeString += "/";
                }
                changeString += arg_vec[1];
                stringIs += changeString;
                //cout << "stringIs2: " << stringIs << endl;
                setenv("PWD", changeString.c_str(), 1);
                //putenv(const_cast<char*>(stringIs.c_str()));
            }
        }
        else {
            // error message
            return Done;
        }
    }

	// repeat

	if (com == "repeat"){
		// if repeat default
		if (arg_vec.size() == 1 && commandList.size() > 2){
			// set command to the previous command
			// 2nd to the last on the command history due to the last command at this point being "repeat"
			command = commandList[commandList.size() - 2];
            /*istringstream iss(command);
            string temp;
            int cc;
            int c2;
            while (iss >> temp) {
                if (temp == "repeat"){
                    c2++;
                }
                cc++;
            }
            if (c2 == 1 && cc >= 2){
                perror("Command Error");
            }
            else{*/
                return Repeat;
            //}
		}
		// if not default repeat
		else if (arg_vec.size() == 2){
			int dCount = 0;
			// checks every char in the string to see if the string is a number
			for (char c : arg_vec[1]){
				if (isdigit(c)){
					dCount++;
				}
			}
			// if the string is indeed a number
			if (arg_vec[1].length() == dCount){
				int a;
				// wrap the string indicating a number in a istringstream to be extracted into an int format
				istringstream iss(arg_vec[1]);
				iss >> a;
				// -1 here because the range of positions in commandList is from 0 to 99. The range of positions indicated by the "history" command is from 1 to 100.
				command = commandList[a - 1];
				return Repeat;

			}
			// if the string is not a number the argument is not in the correct format.
			else {
				cout << "Invalid Command (Non-Digit Num)" << endl;
				return Done;
			}
		}
		else {
			cout << "Invalid Command (repeat)" << endl;
			return Done;
		}
	}

	// history
	if (com == "history"){
		if (arg_vec.size() == 1){
			for (int i = 0; i < commandList.size(); i++){
				cout << i + 1 << ". " << commandList[i] << endl;
			}
		}
		else{
			// to check if number passed is actually a number
			int countr = 0;
			for (char c : arg_vec[1]){
				if (isdigit(c)){
					countr++;
				}
			}
			if (arg_vec[1].length() == countr){
				int c;
				istringstream conv(arg_vec[1]);
				conv >> c;
				int limit = commandList.size() - c;
				for (int i = limit; i < commandList.size(); i++){
					cout << i + 1 << ". " << commandList[i] << endl;
				}
			}
			else{
				cout << "Invalid Argument (history number not specified)" << endl;
				return Done;
			}
		}
	}

	// show
	if (com == "show"){
		map<string, string>::iterator iter;
		if (arg_vec.size() == 1){
			cout << "Variable to Show Missing" << endl;
			return Done;
		}
		if (arg_vec.size() == 2){
			iter = env.find(arg_vec[1]);
			if (iter == env.end()){
				cout << "Variable " << arg_vec[1] << " Not Defined" << endl;
				return Done;
			}
			else {
				cout << iter->first << "= " << iter->second << endl;
			}
		}
		else {
			for (int a = 1; a < arg_vec.size(); a++){
				iter = env.find(arg_vec[a]);
				if (iter == env.end()){
					cout << arg_vec[a] << "= NULL" << endl;
				}
				else {
					cout << iter->first << "= " << iter->second << endl;
				}
			}
		}
	}

	// environ
	if (com == "environ"){
		if (env.size() < 1){
			cout << "No Variables Set" << endl;
			return Done;
		}
		for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it){
			cout << it->first << "=" << it->second << '\n';
		}
	}
	// clr (not done)
	if (com == "clr") {
        // ANSI/VT100 Terminal Control Escape Sequences
        printf("\033[H\033[J");
    }

	// starred are the ones that are done
	// *unset W1, unexport W, chdir W, *exit I, wait I, *echo <comment>, unexport
	if (arg_vec.size() == 2){
		string com = arg_vec[0];
		string com2 = arg_vec[1];
		map<string, string>::iterator iter;
		string::const_iterator it = com2.begin();
		// exit
		if (com == "exit" && isdigit(*it)){
			int value;
			istringstream conv(com2);
			conv >> value;
            isExit = true;
			return value;
		}
		// echo
		if (com == "echo"){
			cout << com2 << endl;
			return Done;
		}
		// unset
		if (com == "unset"){
			iter = env.find(com2);
			if (iter == env.end()){
				cout << "Variable to Unset Missing" << endl;
				return Done;
			}
			else{
				env.erase(com2);
                unsetenv(com2.c_str());
				return Done;
			}
		}
		// unexport
		if (com == "unexport"){
			iter = env.find(com2);
			if (iter == env.end()){
				cout << "Variable to Unexport Missing" << endl;
				return Done;
			}
			else{
				env.erase(com2);
                unsetenv(com2.c_str());
				return Done;
			}
		}
	}

	// set, export
	if (arg_vec.size() == 3){
		string com = arg_vec[0];
		string com2 = arg_vec[1];
		string com3 = arg_vec[2];
		map<string, string>::iterator iter;
		if (com == "set"){
			iter = env.find(com2);
			if (iter == env.end()){
				env.insert(std::pair<string, string>(com2, com3));
			}
			else {
				iter->second = com3;
			}
            setenv(com2.c_str(), com3.c_str(), 1);
		}
		if (com == "export"){
			char* var;
			const char* varString = com2.c_str();
			var = getenv(varString);
			//string check = string(var);
			if (var != NULL){
				env.insert(std::pair<string, string>(com2, com3));
			}
			else {
				cout << "Non Existent Global Variable" << endl;
				return Done;
			}

		}
	}
	return Done;
}