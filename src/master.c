#include "./../include/master_utilities.h"

int main(int argc, char *argv[])
{
  struct stat st = {0};

  if (stat("./out", &st) == -1)
  {
    mkdir("./out", 0700);
  }

  int mode = 0;

  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

  while (mode < 1 || mode > 3)
  {
    printf("Select the mode: \n \t [1] Normal execution (assignment 2) \n \t [2] Server \n \t [3] Client\n");
    scanf("%d", &mode);
  }

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
