/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <poll.h>

using namespace std;

void systemx(const string &cmd)
{
    pid_t child;
    int pipe_fd[2]{};
    auto child_func = [&](){
        dup2(pipe_fd[1], STDOUT_FILENO);
        cout << "I'm Child #" << getpid() << endl;
        execl("/usr/bin/bash", "bash", "-c", cmd.c_str(), nullptr);
        for_each(pipe_fd, pipe_fd+sizeof(pipe_fd), [&](int fd) {
            close(fd);
        });
    };
    
    auto parent_func = [&]() {
        char msg[2048]{};
        struct pollfd pfd { pipe_fd[0], POLLIN, 0};
        cout << "I'm Parent, my child is " << child << endl;
        while (poll(&pfd, 1, 200) > 0) {
            read(pipe_fd[0], msg, sizeof(msg));
            cout << "My child said: " << msg << endl;
        }
        
        waitpid(child, nullptr, 0);
        for_each(pipe_fd, pipe_fd+sizeof(pipe_fd), [&](int fd) {
            close(fd);
        });
    };
    
    pipe(pipe_fd);
    cout << "pipe[read]: " << pipe_fd[0]
         << ", pipe[write]: " << pipe_fd[1] << endl;
    
    child = fork();
    if (child == 0) {
        child_func();
        exit(0);
    }
    else {
        parent_func();
    }
    return;
}

int main()
{
    for (int i = 0; i < 500; i++) {
        systemx("ls -al");
    }
    return 0;
}
