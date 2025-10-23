# unix-shell
ICT374 assignment 2
- [ ] Handle invalid commands / command error.
- [ ] Minimum 100 commands
- [ ] Minimum 1000 arguments in each command
- [x] Built-in prompt that can be reconfigured `% prompt john$` should change the shell prompt to john$
- [x] Built-in command pwd prints the current directory of the shell process
- [x] Directory walk is similar to the bash built-in command cd. Typing the command without a path should set the current directory of the shell to the home directory of the user.
- [ ] Wildcard characters (* or ?) is treated as a filename. The wildcard charcaters in such a token indicate to the shell that the filename must be expanded. (Can be implemented with glob)
- [ ] Redirection of the input using <, >, 2> to push data to command
- [ ] Shell pipeline | 
- [ ] Background job execution
- [ ] Sequential job execution
- [ ] Command history
- [ ] Shell should inherit its environment from parent process
- [x] Use built-in command exit
- [ ] Shell should not terminate by Ctrl C, Ctrl / or Ctrl Z
- [ ] Do not use system function

[The requirements in details can be found here](https://www.it.murdoch.edu.au/~S900432D/oli1l1hsu_x3X64dg72kf7Th973yihbkys9M10in0x/assignments/2025tj-a2/index.shtml)
