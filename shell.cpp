// Shell.cpp : Defines the entry point for the console application.
//

#include "Shell.h"
#include <curses.h>

using namespace std;

static const char*  promptString = "xssh>> ";
vector<string> arg_vec;
string command;

void parse_command(string c){
	istringstream iss(c);
	string result;
	while (iss >> result){
		arg_vec.push_back(result);
	}
}

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

int main(int argc, char * argv[])
{
	while (1){
		printf("%s", promptString);
		fflush(stdout);
		getline(cin, command);
		parse_command(command);
		if (arg_vec.size() <= 0){
			continue;
		}
		// environ, clr, dir, help, cause
		if (arg_vec.size() == 1){
			string com = arg_vec[0];
			if (com == "environ"){

			}
			if (com == "pause"){

			}
			if (com == "clr"){
				  clear();
				   int n;
				   for (n = 0; n < 10; n++)
				     printf( "\n\n\n\n\n\n\n\n\n\n" );
				// for (int i = 1; i < rows; i++){
				// 	printf("\n");
				// }
				// while (1){
				// 	printf("%s", promptString);
				// 	fflush(stdout);
				// 	getline(cin, command);
				// 	parse_command(command);
				// }
				//SetConsoleCursorPosition(hStdOut, homeCoords);
			}
		}
		// show W, unset W1, unexport W, chdir W, *exit I, wait I, *echo <comment>, history n
		if (arg_vec.size() == 2){
			string com = arg_vec[0];
			string com2 = arg_vec[1];
			cout << "com1  " << com << "com2  " << com2 << endl;
			string::const_iterator it = com2.begin();
			if (com == "exit" && isdigit(*it)){
				int value;
				istringstream conv(com2);
				conv >> value;
				return value;
			}
			if (com == "echo"){
				cout << com2 << endl;
			}
			if (com == "show" && isWord(com2)){

			}
		}
		/*if (built_in_command()){

		}*/
		//cout << "size is " << arg_vec.size() << endl;
		arg_vec.clear();
		cin.clear();
	}

	return 0;
}

