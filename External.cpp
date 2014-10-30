#include "External.h"

bool hasAmp;
extern map<string, string> env;
pid_t recentPid;
pid_t waitingPid;
int status;
extern char **environ;

void redirectCommand(string sign, char** cmd1, char** cmd2,int cmd1Size,int cmd2Size)
{
    int fds[2];
    int fd;
    pipe(fds);
    pid_t pid;
    bool stdin_redirect = false;
    bool stdin_append = false;
    bool stdout_redirect = false;
    bool stdout_append = false;

    if (sign=="<" || sign=="<<"){
        stdin_redirect = true;
        if (sign=="<<")
            stdin_append = true;
    } else if (sign==">" || sign==">>"){
        stdout_redirect = true;
        if (sign==">>")
            stdout_append = true;
    }

    if ((recentPid=pid = fork()) == 0) {
        if (stdout_redirect) { // open target file and create it if needed
            if (!stdout_append)
                fd = open(cmd2[0], O_WRONLY | O_CREAT, 0644);
            else
                fd = open(cmd2[0], O_WRONLY | O_APPEND, 0644);
            close(1); // close stdout/
            dup(fd); // dup into stdout
            close(fd); // good practice
        }

        if (stdin_redirect) {
            fd = open(cmd2[0], O_RDONLY);
            //cout << "fd value is " << fd << endl;
            close(0); // close stdin/
            dup2(fd,0); // dup into stdin
            close(fd); // good practice
        }

        execvp(cmd1[0], cmd1);
        exit(1); /* in case execve fails */
    }else {
        if (!hasAmp) {
            waitpid(pid, &status, 0);
        } else
            waitingPid = pid;
    }

}

void pipeCommand(char *const envp[], char** cmd1, char** cmd2,int cmd1Size,int cmd2Size)
{
    int fds[2];
    pipe(fds);
    pid_t pid1, pid2;

    if (pid1 < 0)
        perror("Fork Error");


    if ((recentPid=pid1 = fork()) == 0) {
        // child process cmd2
        dup2(fds[0], 0);
        close(fds[0]);
        close(fds[1]);
        execvp(cmd2[0],cmd2);
        //fflush(stdout);
        perror("execvp failed");

    }
    else if ((recentPid=pid2 = fork()) == 0) {
        // Child process cmd1
        dup2(fds[1], 1);
        close(fds[0]);
        close(fds[1]);

        execvp(cmd1[0],cmd1);
        //fflush(stdout);
        perror("execvp failed");
    } else {
        close(fds[1]);

        if (!hasAmp) {
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        } else {
            waitingPid = pid1;
            waitingPid = pid2;
        }
    }
}
//extern int execvpe(const char* name, char* const argv[], char* const envv[])
//{
//    register const char*  path = name;
//    char          buffer[PATH_MAX];
//
//    if (*path != '/' && !(path = pathpath(buffer, name, NULL, PATH_REGULAR|PATH_EXECUTE)))
//        path = name;
//    execve(path, argv, envv);
//    if (errno == ENOEXEC)
//    {
//        register char**   newargv;
//        register char**   ov;
//        register char**   nv;
//
//        for (ov = (char**)argv; *ov++;);
//        if (newargv = newof(0, char*, ov + 1 - (char**)argv, 0))
//        {
//            nv = newargv;
//            *nv++ = "sh";
//            *nv++ = (char*)path;
//            ov = (char**)argv;
//            while (*nv++ = *++ov);
//            path = pathshell();
//            execve(path, newargv, envv);
//            free(newargv);
//        }
//        else
//            errno = ENOMEM;
//    }
//    return -1;
//}

void runExternal(char * envp[], int cmdSize, char ** cstrCmd)
{
    pid_t pid;

    if ((recentPid=pid = fork()) == 0) {
        fflush(stdout);
        //execvp(cstrCmd[0],cstrCmd);
        char **old_env = environ;
        //environ = envp;
        execvp(cstrCmd[0], cstrCmd);
        // Oops - must have failed!
        environ = old_env;
        perror("execvp error");
        exit(1);
    } else{
        if (pid < 0)
            perror("Fork Error");

        if (!hasAmp){
            waitpid(pid, &status, 0);
        }
        else waitingPid = pid;
    }
}

void threeCommands(string sign[], char** cmd1, char** cmd2,char** cmd3,int cmd1Size,int cmd2Size,int cmd3Size){

    int pipes[4];
    pipe(pipes);
    pipe(pipes+2);
    pid_t pid1, pid2, pid3;

    pid1 = fork();
    recentPid = pid1;
    if (pid1 < 0)
        perror("Fork Error");
    if (pid1 == 0)
    {
        if (sign[0] == ">" || sign[0] == ">>") {
            if (sign[0] == ">")
                pipes[1] = open(cmd2[0], O_WRONLY | O_CREAT, 0644);
            else
                pipes[1] = open(cmd2[0], O_WRONLY | O_APPEND, 0644);
        }

        if (sign[0] == "<" || sign[0] == "<<") {
            pipes[0] = open(cmd2[0], O_RDONLY);
            dup2(pipes[0], 0); // dup into stdin
        }

        dup2(pipes[1], 1);

        close(pipes[0]);
        close(pipes[1]);
        close(pipes[2]);
        close(pipes[3]);
        execvp(cmd1[0], cmd1);
        fflush(stdout);
        perror("execvp failed (1)");
        exit(1);
    }
    else
    {
        if ((recentPid=pid2 = fork()) == 0) {
            if (sign[1] == ">" || sign[1] == ">>") {
                if (sign[1] == ">")
                    pipes[3] = open(cmd3[0], O_WRONLY | O_CREAT, 0644);
                else
                    pipes[3] = open(cmd3[0], O_WRONLY | O_APPEND, 0644);
            }

            if (sign[1] == "<" || sign[1] == "<<") {
                pipes[0] = open(cmd3[0], O_RDONLY);
            }
            dup2(pipes[0], 0);
            dup2(pipes[3], 1);


            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);

            if (sign[0] == "|"){
                execvp(cmd2[0], cmd2);
                perror("execvp failed (2)");
            }
            else {
                execlp("cat", "cat", NULL);
            }

            fflush(stdout);
            exit(1);
        }
        else
        {
            if ((recentPid=pid3=fork()) == 0)
            {
                // replace cut's stdin with input read of 2nd pipe
                dup2(pipes[2], 0);

                // close all ends of pipes
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);

                if (sign[1] == "|"){
                    execvp(cmd3[0], cmd3);
                    perror("execvp failed (3)");
                }
                execlp("cat", "cat", NULL);
                fflush(stdout);
                exit(1);
            }
            else
            {
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);
                if (!hasAmp) {
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
                    waitpid(pid3, &status, 0);
                } else {
                    waitingPid = recentPid;
                }
            }
        }
    }
}
void fourCommands(string sign[], char** cmd1, char** cmd2,char** cmd3,char** cmd4) {
    int pipes[6];
    pipe(pipes);
    pipe(pipes+2);
    pipe(pipes+4);
    pid_t pid1, pid2, pid3, pid4;

    pid1 = fork();
    recentPid = pid1;
    if (pid1 < 0)
        perror("Fork Error");
    if (pid1 == 0)
    {
        // child process cmd2
        if (sign[0] == ">" || sign[0] == ">>") {
            if (sign[0] == ">")
                pipes[1] = open(cmd2[0], O_WRONLY | O_CREAT, 0644);
            else
                pipes[1] = open(cmd2[0], O_WRONLY | O_APPEND, 0644);
        }

        if (sign[0] == "<" || sign[0] == "<<") {
            pipes[0] = open(cmd2[0], O_RDONLY);
            dup2(pipes[0], 0); // dup into stdin
        }

        dup2(pipes[1], 1);

        close(pipes[0]);
        close(pipes[1]);
        close(pipes[2]);
        close(pipes[3]);
        close(pipes[4]);
        close(pipes[5]);
        execvp(cmd1[0], cmd1);
        fflush(stdout);
        perror("execvp failed (1)");
    }
    else {
        if ((recentPid=pid2 = fork()) == 0) {
            if (sign[1] == ">" || sign[1] == ">>") {
                if (sign[1] == ">")
                    pipes[3] = open(cmd3[0], O_WRONLY | O_CREAT, 0644);
                else
                    pipes[3] = open(cmd3[0], O_WRONLY | O_APPEND, 0644);
            }

            if (sign[1] == "<" || sign[1] == "<<") {
                pipes[0] = open(cmd3[0], O_RDONLY);
            }
            dup2(pipes[0], 0);
            dup2(pipes[3], 1);

            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);
            close(pipes[4]);
            close(pipes[5]);

            if (sign[0] == "|") {
                execvp(cmd2[0], cmd2);
                perror("execvp failed (2)");
            }
            else {
                execlp("cat", "cat", NULL);
            }

            fflush(stdout);
            exit(1);
        }
        else {
            if ((recentPid=pid3 = fork()) == 0) {
                if (sign[2] == ">" || sign[2] == ">>") {
                    if (sign[2] == ">")
                        pipes[5] = open(cmd4[0], O_WRONLY | O_CREAT, 0644);
                    else
                        pipes[5] = open(cmd4[0], O_WRONLY | O_APPEND, 0644);
                }

                if (sign[2] == "<" || sign[2] == "<<") {
                    pipes[2] = open(cmd3[0], O_RDONLY);
                }

                // replace cut's stdin with input read of 2nd pipe
                dup2(pipes[2], 0);
                dup2(pipes[5], 1);

                // close all ends of pipes
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);
                close(pipes[4]);
                close(pipes[5]);
                if (sign[1] == "|") {
                    execvp(cmd3[0], cmd3);
                    perror("execvp failed (3)");
                } else
                    execlp("cat", "cat", NULL);
                fflush(stdout);
                exit(1);
            }
            else {
                if ((recentPid=pid4 = fork()) == 0) {
                    dup2(pipes[4], 0);

                    close(pipes[0]);
                    close(pipes[1]);
                    close(pipes[2]);
                    close(pipes[3]);
                    close(pipes[4]);
                    close(pipes[5]);
                    if (sign[2] == "|") {
                        execvp(cmd4[0], cmd4);
                        perror("execvp failed (4)");
                    } else {
                        execlp("cat", "cat", NULL);
                    }
                    perror("execvp failed (4)");
                    fflush(stdout);
                    exit(1);
                } else {
                    close(pipes[0]);
                    close(pipes[1]);
                    close(pipes[2]);
                    close(pipes[3]);
                    close(pipes[4]);
                    close(pipes[5]);
                    if (!hasAmp){
                        waitpid(pid1, &status, 0);
                        waitpid(pid2, &status, 0);
                        waitpid(pid3, &status, 0);
                        waitpid(pid4, &status, 0);
                    } else {
                        waitingPid = recentPid;
                    }
                }
            }
        }
    }
}

void checkPipesNumber(vector<string> argvc)
{
    int MAX_OPERATORS=3;

    int countPipes=0;
    //PIPESTATE STATE = NEITHER;
    int indexes[4]={0};
    string sign[4];

    //checks Amp
    hasAmp = false;
    if (argvc[argvc.size()-1] == "&"){
        hasAmp = true;
        argvc.pop_back();

    }

    for(int i = 0; i < argvc.size(); i++){
        if (argvc[i] == "|" || argvc[i] == "<" || argvc[i] == ">" || argvc[i] == "<<" || argvc[i] == ">>") {
            sign[countPipes]=argvc[i];
            indexes[countPipes] = i;
            countPipes++;
        }
    }

    int idx = 0;
    char *envVari[env.size()+1];
    // Saves env variables as a c string char array
    for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it){
        //cout << it->first << "=" << it->second << '\n';
        char *strChar;
        strChar = new char[it->first.size()+it->second.size()+1];
        string temp = it->first + "=" + it->second;
        strcpy(strChar,temp.c_str());
        envVari[idx] = strChar;
        idx++;
    }
    int cmd1Size;

    cmd1Size = argvc.size();
    char *cstrCmd[cmd1Size + 1];
    for (unsigned int i = 0; i < cmd1Size; i++) {
        //save right hand side argv in c style format for execlv
        char *cstr;
        cstr = new char[argvc[i].size() + 1];
        strcpy(cstr, argvc[i].c_str());
        cstrCmd[i] = cstr;
    }
    cstrCmd[cmd1Size] = 0;
\
    int cmd2Size;
    int cmd3Size;
    int cmd4Size;

    switch (countPipes){
        case 0: {
            runExternal(envVari,cmd1Size,cstrCmd);
            break;
        }
        case 1:
        {
            cmd1Size = indexes[0];
            cmd2Size = argvc.size()-indexes[0]-1;

            char *cstrCmd1[cmd1Size+1];
            char *secondCmd[cmd2Size+1];

            for (unsigned int i = 0; i < indexes[0]; i++)
            {
                //save left hand side argv in c style format for execlv
                char *cstr;
                cstr = new char[argvc[i].size()+1];
                strcpy(cstr,argvc[i].c_str());
                cstrCmd1[i] = cstr;

            }
            cstrCmd1[cmd1Size] = 0;

            for (unsigned int i = indexes[0]+1; i < argvc.size(); i++)
            {
                //save right hand side argv in c style format for execlv
                char *secondStr;
                secondStr = new char[argvc[i].size()+1];
                strcpy(secondStr,argvc[i].c_str());
                secondCmd[i - indexes[0]-1] = secondStr;
            }
            secondCmd[cmd2Size] = 0;

            if (sign[0] == "|"){
                pipeCommand(envVari,cstrCmd1,secondCmd,cmd1Size,cmd2Size);
            } else if (sign[0] == "<" || sign[0]  == ">" || sign[0]  == "<<" || sign[0]  == ">>"){
                redirectCommand(sign[0],cstrCmd1,secondCmd,cmd1Size,cmd2Size);
            }
            break;
        }
        case 2:
        {
            cmd1Size = indexes[0];
            cmd2Size = indexes[1]-indexes[0]-1;
            cmd3Size = argvc.size()-indexes[1]-1;

            char *cmd1[cmd1Size+1];
            char *cmd2[cmd2Size+1];
            char *cmd3[cmd3Size+1];

            for (unsigned int i = 0; i < indexes[0]; i++)
            {
                //save left hand side argv in c style format for execlv
                char *cstr;
                cstr = new char[argvc[i].size()+1];
                strcpy(cstr,argvc[i].c_str());
                cmd1[i] = cstr;

            }
            cmd1[cmd1Size] = 0;

            for (unsigned int i = indexes[0]+1; i < indexes[1]; i++)
            {
                //save right hand side argv in c style format for execlv
                char *secondStr;
                secondStr = new char[argvc[i].size()+1];
                strcpy(secondStr,argvc[i].c_str());
                cmd2[i - indexes[0]-1] = secondStr;
            }
            cmd2[cmd2Size] = 0;

            for (unsigned int i = indexes[1]+1; i < argvc.size(); i++)
            {
                //save right hand side argv in c style format for execlv
                char *thirdStr;
                thirdStr = new char[argvc[i].size()+1];
                strcpy(thirdStr,argvc[i].c_str());
                cmd3[i - indexes[1]-1] = thirdStr;
            }
            cmd3[cmd3Size] = 0;

            threeCommands(sign, cmd1, cmd2, cmd3, cmd1Size, cmd2Size, cmd3Size);
            break;
        }
        case 3: {
            cmd1Size = indexes[0];
            cmd2Size = indexes[1] - indexes[0] - 1;
            cmd3Size = indexes[2] - indexes[1] - 1;
            cmd4Size = argvc.size() - indexes[2] - 1;


            char *cmd1[cmd1Size+1];
            char *cmd2[cmd2Size+1];
            char *cmd3[cmd3Size+1];
            char *cmd4[cmd4Size+1];

            for (unsigned int i = 0; i < indexes[0]; i++)
            {
                //save left hand side argv in c style format for execlv
                char *cstr;
                cstr = new char[argvc[i].size()+1];
                strcpy(cstr,argvc[i].c_str());
                cmd1[i] = cstr;

            }
            cmd1[cmd1Size] = 0;

            for (unsigned int i = indexes[0]+1; i < indexes[1]; i++)
            {
                //save right hand side argv in c style format for execlv
                char *secondStr;
                secondStr = new char[argvc[i].size()+1];
                strcpy(secondStr,argvc[i].c_str());
                cmd2[i - indexes[0]-1] = secondStr;

            }
            cmd2[cmd2Size] = 0;

            for (unsigned int i = indexes[1]+1; i < indexes[2]; i++)
            {
                //save right hand side argv in c style format for execlv
                char *thirdStr;
                thirdStr = new char[argvc[i].size()+1];
                strcpy(thirdStr,argvc[i].c_str());
                cmd3[i - indexes[1]-1] = thirdStr;

            }
            cmd3[cmd3Size] = 0;

            for (unsigned int i = indexes[2]+1; i < argvc.size(); i++)
            {
                //save right hand side argv in c style format for execlv
                char *fourthStr;
                fourthStr = new char[argvc[i].size()+1];
                strcpy(fourthStr,argvc[i].c_str());
                cmd4[i - indexes[2]-1] = fourthStr;
            }
            cmd4[cmd4Size] = 0;
            fourCommands(sign, cmd1 , cmd2, cmd3, cmd4);
            break;
        }
        default:
            perror("XSSH does not support more than 3 pipes!");
            break;
    }
}