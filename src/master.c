#include "./../include/master_utilities.h"

int main(int argc, char *argv[])
{
  if (signal(SIGUSR1, disconnect_handler) == SIG_ERR)
    printf("\ncan't catch SIGUSR1\n");

  if (signal(SIGUSR2, exit_handler) == SIG_ERR)
    printf("\ncan't catch SIGUSR2\n");

  int mode = 0;

  while (change_mode == 1 || disconnect == 1)
  {
    if (change_mode == 1)
    {
      change_mode = 0;
      mode = 0;
      mode = ask_mode(mode);
      last_mode = mode;
    }
    else if (disconnect == 1)
    {
      disconnect = 0;
      mode = last_mode;
    }
    if (execute(mode) == -1)
    {
      return -1;
    }
  }

  return 0;
}
