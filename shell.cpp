// Shell.cpp : Defines the entry point for the console application.
//
#ifdef WINDOWS
	#include "stdafx.h"
#endif

//#include <tclDecls.h>
#include "Shell.h"
//#define _XOPEN_SOURCE

using namespace std;

extern vector<string> arg_vec;
extern deque<string> commandList;
extern string command;
extern map<string, string> env;
extern char* dPath;
extern pid_t recentPid;
extern bool isExit;
extern bool hasAmp;
bool firstRun=true;
bool xFlag = false;
int dFlag = 0;
char* hPath;

static const char*  promptString = "xssh>> ";
static volatile sig_atomic_t sig_caught = 0;
void(*int_handler)(int);
void(*abrt_handler)(int);
void(*alrm_handler)(int);
void(*hup_handler)(int);
void(*term_handler)(int);
void(*quit_handler)(int);
void(*stop_handler)(int);
void(*cont_handler)(int);
void(*usr1_handler)(int);
void(*usr2_handler)(int);

void sig_handler(int signum){
	if (signum == SIGINT){
		int_handler = signal(SIGINT, sig_handler);
        if(recentPid>0)
		    kill(recentPid, signum);
        cout << endl;
	}
	if (signum == SIGABRT){
		abrt_handler = (SIGABRT, sig_handler);
	}
	if (signum == SIGTERM){
		term_handler = signal(SIGTERM, sig_handler);
	}
	if (signum == SIGALRM){
		alrm_handler = signal(SIGALRM, sig_handler);
	}
    if (signum == SIGQUIT){
        quit_handler = signal(SIGQUIT, sig_handler);
        kill(recentPid, signum);
        cout << endl;
    }
    if (signum == SIGCONT){
        cont_handler = signal(SIGCONT, sig_handler);
        if(recentPid>0)
            kill(recentPid, signum);
        cout << endl;
    }
    if (signum == SIGSTOP){
        stop_handler = signal(SIGSTOP, sig_handler);
        if(recentPid>0)
            kill(recentPid, signum);
        cout << endl;
    }
    if (signum == SIGUSR1){
        usr1_handler = signal(SIGUSR1, sig_handler);
    }
    if (signum == SIGUSR2){
        usr2_handler = signal(SIGUSR2, sig_handler);
    }
    if (signum == SIGHUP){
        hup_handler = signal(SIGHUP, sig_handler);
    }
}

void initialize(){
    // shell
    char* pat = getenv("PWD");
    string s(pat);
    env.insert(pair<string, string>("home", s));
    s+="/xssh";
    env.insert(pair<string, string>("shell", s));

    // Path
    char* pat2 = getenv("PATH");
    string s2(pat2);
    env.insert(pair<string, string>("PATH", s2));

    //Parent
    env.insert(pair<string, string>("parent", s));

    setenv("shell", s.c_str(), 1);
    setenv("home", s.c_str(), 1);
    setenv("parent", s.c_str(), 1);


}

bool checkInputParams(int argc, char * argv[]){
    bool hasParam = false;

    for (int i = 0; i < argc; i++){
        char fFlags[3];
        strcpy(fFlags, "-f");
        char xFlags[3];
        strcpy(xFlags, "-x");
        char dFlags[3];
        strcpy(dFlags, "-d");

        if (strcmp(fFlags, argv[i])==0){
            hasParam = true;
            int fd;
            pipe(&fd);
            pid_t pid;
            fd = open(argv[i+1], O_RDONLY);
            dup2(fd, 0); // dup into stdin

            close(fd);
            fflush(stdout);

            for (int j=i+2; j < argc; j++){
                int index = j-i-1;
                string keyEvn(argv[i]);
                string indexS = to_string(index);
                string valEvn= "$"+indexS;
                env.insert(pair<string, string>(keyEvn, valEvn));
            }
        }

        if (strcmp(xFlags, argv[i])==0){
            xFlag = true;
        }

        /*if (strcmp(dFlags, argv[i])==0){
            int a = argv[i+1];
            char buf [3];
            itoa(a,buf,10);
            dFlag = argv[i+1];
        }*/
    }
    firstRun = false;
    return hasParam;
}

int main(int argc, char * argv[])
{
    int sin_cpy = dup(0);
	hPath = getenv("PWD");
	int_handler = signal(SIGINT, sig_handler);
	abrt_handler = signal(SIGABRT, sig_handler);
	term_handler = signal(SIGTERM, sig_handler);
	alrm_handler = signal(SIGALRM, sig_handler);
    quit_handler = signal(SIGQUIT, sig_handler);
    cont_handler = signal(SIGCONT, sig_handler);
    stop_handler = signal(SIGSTOP, sig_handler);
    usr1_handler = signal(SIGUSR1, sig_handler);
    usr2_handler = signal(SIGUSR2, sig_handler);
    hup_handler = signal(SIGHUP, sig_handler);

    initialize();

	while (1){
		cin.clear();

        printf("%s", promptString);
		fflush(stdout);
        if (firstRun) {
            checkInputParams(argc, argv);
        } else {
            dup2(sin_cpy, 0);
        }
		getline(cin, command);
        // Utilizing label to be used for the repeat command.
        label:
		if (command.length() < 1) {
            continue;
        }

		parse_command(command);

		if (built_in_command(arg_vec[0])){
			int result = exe(arg_vec);
            if (isExit){
                return result;
            }
			if (result == Pause){
                raise(SIGSTOP);
                cin.clear();
                string temp;
                getline(cin, temp);
                while (command != "") {
                    getline(cin, temp);
                }
                raise(SIGCONT);
                continue;
			}
			else if (result == Repeat){
                arg_vec.clear();
                goto label;
            }
            else if (result == Done){
                arg_vec.clear();
                continue;
			}
		} else {
            checkPipesNumber(arg_vec);
        }
        //close(1);
        //dup2(0,0);
		arg_vec.clear(arg_vec);
	}

	return 0;
}

