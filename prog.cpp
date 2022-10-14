/**
 * Assignment 2: Simple UNIX Shell
 * @file pcbtable.h
 * @author Elaeth Lilagan & Kyrstn Hall
 * @brief This is the main function of a simple UNIX Shell. You may add
 * additional functions in this file for your implementation
 * @version 0.1
 */
// You must complete the all parts marked as "TODO". Delete "TODO" after you are
// done. Remember to add sufficient and clear comments to your code

#include <cstring> 
#include <fcntl.h> 
#include <iostream> 
#include <stdio.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <vector> 
#include <cstdlib> 
#include <errno.h>
#include <sys/types.h> 

using namespace std;

#define MAX_LINE 80 // The maximum length command
bool ampersand = false;  // flag to determine if command contains &

/*Prototypes were needed cause functions weren't able to be read or accessed*/
void in_redirect(char *file);
void out_redirect(char *file);
int parse_command(char command[], char *args[]);
void execArgs(char command[], char *args[]);

// < , sort < in.txt , file will serve as input to the sort
void in_redirect(char *file) 
{
  int i = open(file, O_RDONLY);
  dup2(i, 0);
  close(i);
}

// > , ls > t.txt , output from ls will be in file t.txt
void out_redirect(char *file) 
{
  int o = open(file, O_WRONLY | O_APPEND | O_TRUNC | O_CREAT, 0200);
  dup2(o, 1);
  close(o);
}

/**
 * @brief parse out the command and arguments from the input command separated
 * by spaces
 *
 * @param command
 * @param args
 * @return int
 */
int parse_command(char command[], char *args[]) {
  // TODO: implement this function
   int count = 0; // to track the array/how many arguments there are
   char *tok = strtok(command, " "); // takes first argument of the command, stopping at first space
   while (tok != NULL) // going through the command
     {
      if(*tok == '<') //checking if the character is an input
        in_redirect(strtok(NULL, " ")); //call the input redirect function, next arg is file (passed)
      else if(*tok == '>') //checking if the character is an output
        out_redirect(strtok(NULL, " ")); //call the output redirection function, next arg is file (passed)
      else if(*tok == '|') //checks if there is a pipe
      {
        args[count - 1] = NULL; //set the args array to NULL
        execArgs(command, args);//call the function to run the pipe
        count = 1; //set to one once done
      }
      else//if there are no special cases or characters
      {
        args[count] = tok; // argument stored in args
        count++;
      }
      tok = strtok(NULL, " "); // take off the spaces and set to NULL
    }
  
    // need to check here if command has &
    if (strcmp(args[count - 1], "&") == 0) // there is an ampersand, strcmp returns 0 if 2 strings are equal
    {
      args[count - 1] = NULL; // replaces & with NULL
      ampersand = true;          // flag, has an ampersand
    } 
    else //if there is no ampersand
    {
      args[count] = NULL;
      ampersand = false;
    }
    return count; // return the index of the array, the number of arguments ?
}

/**
  Execution function for all cases within the shell
 */ 
void execArgs(char command[], char *args[]) 
{
  pid_t pid, child1, child2;
  int fd[2];//
  pid = fork();  // step 1

  if(pipe(fd))//set if there is a pipe located in the command line
  {
    perror("Error:\tAbort\n");
    abort();
  }
  /* Performs for the first child */
  switch (child1 = fork())
  {
    //if statement wasnt able to be performed due to using the result of an assignment as a condition without parentheses so it will be performed using switch which satisfies the child1 = fork()
    case -1://if the child1 <= 0
      perror("Error:\tAbort\n");
      abort();//abort since the error has been caught
      break;
    case 0:  /* child 1 gets called */
      close(fd[0]);//close the first child -- pipe file descriptor
      //write(pipefds[1], "Hello, Friend!", 10);
      exit(0);//exit once the child1 is called
    default: /* parent */
      ;//do nothing
  }
  /* Performs for the second child */
  switch (child1 = fork())
  {
    case -1:// if the child1 <= 0
      perror("Error:\tAbort\n");
      abort();//abort since the error has been caught
      break;
    case 0: 
      //char buffer[MAX_LINE];//same as command, just declared it in the execArgs func
      ssize_t nread;//signed size t integer that takes into account of accessing 32 bit data 

      close(fd[1]);//close the second child -- pipe file descriptor
      nread = read(fd[0], command, sizeof(*command) - 1);//perform the pipe that will be communicated with
      //nread = read(fd[0], buffer, sizeof(buffer) - 1);//perform the pipe that will be communicated with
      if(nread < 0)//same condition as case -1:
      {
        perror("Error:\tAbort\n");
        abort();//abort if condition is met
      }
      else
      {
        command[nread] = '\0'; //terminate character string
      }
      exit(0);
    default:
      ;//do nothing
  }
  close(fd[0]); //close first child
  close(fd[1]); //close second child
  if(child1 >= 0) //if there is still data from first child
  {
    wait(NULL);//set it to wait stage
    if(child2 >= 0) //if there is still data from second child
    {
      wait(NULL);//set it to wait stage
    }
  }
  
  /* used when a pipe is not in the command and when pipe is done */
  if (pid < 0) // error 
  {
    printf("Fork Failed");
    exit(1);//terminate the program once condition is hit
  } 
  else if (pid == 0) //  step 2 - child process
  {
    execvp(args[0], args); // execute command
    
    if (execvp(args[0], args) < 0) // execvp returns neg value if doesnt run
    {
      cout << "Command not found" << endl;
      exit(1);//terminate the program once condition is hit
    }
  }
  else //  step 3 - parent process
  {
    if (ampersand == false) // command does not have &, parent will invoke wait
    {
      wait(NULL);//parent is in waiting stage
    }
  }
}
  
// TODO: Add additional functions if you need

/**
 * @brief The main function of a simple UNIX Shell. You may add additional
 * functions in this file for your implementation
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @return The exit status of the program
 */
int main(int argc, char *argv[])
{
  char command[MAX_LINE];       // the command that was entered
  char *args[MAX_LINE / 2 + 1]; // parsed out command line arguments
  int should_run = 1;           /* flag to determine when to exit program */
  vector<string> history;      // to store the commmands for history

  // TODO: Add additional variables for the implementation.

  while (should_run) //when the program is not determined to be exited
  {
    printf("osh> ");
    fflush(stdout);
    // Read the input command
    fgets(command, MAX_LINE, stdin);
    command[strlen(command) - 1] = '\0'; // need to make last index of char array to \0

    if(strncmp(command, "exit", 4) == 0) // if user enters exit & includes the space after
        should_run = 0;
    else if(strcmp(command, "!!") == 0) // displays last command, executes it
    {
      if(history.empty()) // vector is empty
        cout << "No command history" << endl;
      else 
      {
        string prev_command = history[history.size()-1]; // previous should be last index of vector
        cout << prev_command << endl;
        history.push_back(prev_command); // pushing command to vector
        strcpy(command, history[history.size()-1].c_str()); // copies previous command to command
        int num_args = parse_command(command, args); // parse command
        execArgs(command, args); // executes command
      }
    }
    else // normal command
    { 
      history.push_back(command); // pushing command to vector

      // if & is found in the command, and the index before has no space, ex ls&
      // add space between command and &
      for(int i=0; i<strlen(command); i++)
      {
        if((command[i] == '&') && (command[i-1] != ' ')) // i=symbol, index before doesnt have space, so add space
        {
          command[i] = ' '; // current index is replaced with a space
          command[i+1] = '&'; // ampersand placed after space
          command[i+2] = '\0'; // last index will be NULL
        }
      }
      int num_args = parse_command(command, args); // Parse the input command
      execArgs(command, args); // executing command

      // TODO: Add your code for the implementation
      /**
       * After reading user input, the steps are:
       * (1) fork a child process using fork()
       * (2) the child process will invoke execvp()
       * (3) parent will invoke wait() unless command included &
       */
      // done in execArg()
    }
  }   // end of while
  return 0;
}