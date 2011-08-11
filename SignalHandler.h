#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

using namespace std;

class SignalHandler
{
public:
  SignalHandler();
  static sig_atomic_t getExitSignal();
  void init();

private:
  static void handleSigterm(sig_atomic_t signo);
  static void handleSigusr1(sig_atomic_t signo);

  static int exit_signal;
};

#endif // SIGNAL_HANDLER_H
