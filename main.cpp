#include "main.h"

using namespace std;

void reaper(int sig) {
	int status;
	waitpid(-1, &status, WNOHANG);
}

void prompt() {
	cout << "＼(・ω・＼)SAN値！(／・ω・)／ピンチ！ ";
}

void init() {
	(void) signal(SIGCHLD, reaper);
	//`signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_DFL);
}

void executeSingleCommand(char **argv, int in, int out, int pipeSize, vector<UnNamedPipe> pipeCtrl, char *inputFile, char *outputFile) {
	pid_t pid;
	pid = fork();

	if(pid < 0) {
		cout << "fork error" << endl;
		exit(1);
	}
	/* child */
	else if(pid == 0) {
		/* input & output */
		if(in != STDIN_FILENO) { dup2(in, STDIN_FILENO); }
		else if(inputFile) {
			int input = open(inputFile, O_RDONLY);
			if(input == -1) {
				cout << "read file error" << endl;
				exit(1);
			}
			dup2(input, STDIN_FILENO);
			close(input);
		}
		if(outputFile) {
			int output = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if(output == -1) {
				cout << "write file error" << endl;
				exit(1);
			}
			dup2(output, STDOUT_FILENO);
			close(output);
		}
		else if(out != STDOUT_FILENO) { dup2(out, STDOUT_FILENO); }
		/* close pipe */
		for(int i = 0;i < pipeSize;i++) {
			pipeCtrl[i].closeReadPipe();
			pipeCtrl[i].closeWritePipe();
		}
		/* exec */
		execvp(argv[0], argv);
		printf("Unknown command: [%s].\n", argv[0]);
		exit(1);
	}
}

bool execute(vector<Command> commands) {
	int commandsSize = commands.size(), pipeSize = 0;
	for(int i = 0;i < commandsSize;i++) {
		if(commands[i].type() == 1) { pipeSize++; }
	};
	vector<UnNamedPipe> pipeCtrl(pipeSize, UnNamedPipe());
	for(int i = 0;i < pipeSize;i++) { pipeCtrl[i].createPipe(); };

	/* Each command */
	for(int i = 0;i < commandsSize;i++) {
		int in, out;
		int type = commands[i].type(),
			nextType = i < commandsSize - 1 ? commands[i+1].type() : -1,
			afterNextType = i < commandsSize - 2 ? commands[i+2].type() : -1;
		char **argv = commands[i].genArgs(),
			 **nextArgv = i < commandsSize - 1 ? commands[i+1].genArgs() : NULL,
			 **afterNextArgv = i < commandsSize - 2 ? commands[i+2].genArgs() : NULL;
		string cmd(argv[0]);
		/* input & output */
		in = i == 0 ? STDIN_FILENO : pipeCtrl[i - 1].getReadPipe();
		if((nextType == 2 && afterNextType == 3) || (nextType == 3 && afterNextType == 2)) {
			i += 2;
		}
		else if(nextType == 2 || nextType == 3) {
			i++;
		}
		out = i == commandsSize - 1 ? STDOUT_FILENO : pipeCtrl[i].getWritePipe();
		/* execute command */
		if(cmd == "exit") {
			return false;
		}
		else if(cmd == "export") {
			if(setenv(argv[1], argv[2], 1) != 0) {
				cout << "export error" << endl;
			}
		}
		else if(cmd == "unset") {
			if(unsetenv(argv[1]) != 0) {
				cout << "unset error" << endl;
			}
		}
		char *inputFile = nextType == 2 ? nextArgv[0] : afterNextType == 2 ? afterNextArgv[0] : NULL;
		char *outputFile = nextType == 3 ? nextArgv[0] : afterNextType == 3 ? afterNextArgv[0] : NULL;
		executeSingleCommand(argv, in, out, pipeSize, pipeCtrl, inputFile, outputFile);
	}
	/* close pipe */
	for(int i = 0;i < pipeSize;i++) {
		pipeCtrl[i].closeReadPipe();
		pipeCtrl[i].closeWritePipe();
	}
	/* wait */
	for(int i = 0;i < pipeSize + 1;i++) {
		int status;
		wait(&status);
	}
	return true;
}

int main(int argc, char **argv) {
	init();
	while(1) {
		/* prompt */
		prompt();

		string input;
		getline(cin, input);
		Commands command(input);
		if(!execute(command.getCommands())) {
			break;
		}
	}
}
