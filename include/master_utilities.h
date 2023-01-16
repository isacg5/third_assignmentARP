#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX 256

int change_mode = 1;
int disconnect = 1;
int last_mode = 0;

int spawn(const char *program, char *arg_list[])
{

  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    return 1;
  }

  else if (child_pid != 0)
  {
    return child_pid;
  }

  else
  {
    if (execvp(program, arg_list) == 0)
    {
      perror("Exec failed");
      return -1;
    }
  }
}

int normal_mode(char *arg_list_A[], char *arg_list_B[])
{
  pid_t pid_proc_A, pid_proc_B;
  printf("Normal mode selected!\n");
  if ((pid_proc_A = spawn("/usr/bin/konsole", arg_list_A)) == -1)
  {
    return -1;
  }

  usleep(1000000);

  if ((pid_proc_B = spawn("/usr/bin/konsole", arg_list_B)) == -1)
  {
    return -1;
  }

  int status;
  waitpid(pid_proc_A, &status, 0);
  waitpid(pid_proc_B, &status, 0);
  printf("Main program exiting with status %d\n", status);
  return status;
}

int server_mode(char *arg_list_B[])
{
  pid_t pid_proc_server, pid_proc_B;
  char port[MAX];
  char address[MAX];

  printf("Server mode selected!\n");
  printf("Introduce the address: \n");
  scanf("%s", address);
  printf("Introduce the port: \n");
  scanf("%s", port);
  printf("[SERVER] Establishing connection with address %s in port %s...\n", address, port);

  char *arg_list_server[] = {"/usr/bin/konsole", "-e", "./bin/server", address, port, NULL};

  if ((pid_proc_server = spawn("/usr/bin/konsole", arg_list_server)) == -1)
  {
    return -1;
  }

  sleep(10);

  if ((pid_proc_B = spawn("/usr/bin/konsole", arg_list_B)) == -1)
  {
    return -1;
  }

  int status_server;
  waitpid(pid_proc_server, &status_server, 0);
  waitpid(pid_proc_B, &status_server, 0);
  printf("Main program exiting with status %d\n", status_server);
  return status_server;
}

int client_mode()
{
  pid_t pid_proc_client;
  char port[MAX];
  char address[MAX];

  printf("Client mode selected!\n");
  printf("Introduce the address: \n");
  scanf("%s", address);
  printf("Introduce the port: \n");
  scanf("%s", port);
  printf("[CLIENT] Establishing connection with address %s in port %s...\n", address, port);

  char *arg_list_client[] = {"/usr/bin/konsole", "-e", "./bin/client", address, port, NULL};
  if ((pid_proc_client = spawn("/usr/bin/konsole", arg_list_client)) == -1)
  {
    return -1;
  }

  int status_client;
  waitpid(pid_proc_client, &status_client, 0);
  printf("Main program exiting with status %d\n", status_client);
  return status_client;
}

int ask_mode(int mode)
{
  while (mode < 1 || mode > 3)
  {
    printf("Select the mode: \n \t [1] Normal execution (assignment 2) \n \t [2] Server \n \t [3] Client\n");
    scanf("%d", &mode);
  }
  return mode;
}

void exit_handler(int signo)
{
  if (signo == SIGUSR2)
  {
    change_mode = 1;
  }
}

void disconnect_handler(int signo)
{
  if (signo == SIGUSR1)
  {
    disconnect = 1;
  }
}

int execute(int mode)
{
  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

  switch (mode)
  {
  case 1:
    int status;
    if ((status = normal_mode(arg_list_A, arg_list_B)) == -1)
    {
      return -1;
    }
    break;

  case 2:
    int status_server;
    if ((status_server = server_mode(arg_list_B)) == -1)
    {
      return -1;
    }
    break;

  case 3:
    int status_client;
    if ((status_client = client_mode()) == -1)
    {
      return -1;
    }
    break;
  }

  return 0;
}