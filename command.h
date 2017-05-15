#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "pipe.h"

using namespace std;

class Command {
	private:
		char symbol;
		vector<string> args;
		void splitCommand(string input);
	public:
		Command(char symbol, string input);
		int type();
		char **genArgs();
};

class Commands {
		private:
		// | < >
		string symbol;
		vector<Command> commands;
		void splitSymbol(string input);
		string trimCommand(string input);
		public:
		Commands(string input);
		vector<Command> getCommands();
};
