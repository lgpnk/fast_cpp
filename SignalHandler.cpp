#include <signal.h>
#include <unistd.h>

#include "Shared.h"
#include "SignalHandler.h"

sig_atomic_t SignalHandler::exit_signal;

SignalHandler::SignalHandler()
{
}

sig_atomic_t
SignalHandler::getExitSignal()
{
  return exit_signal;
}

void SignalHandler::init()
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handleSigterm;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handleSigusr1;
    sigaction(SIGUSR1, &sa, NULL);
}

void SignalHandler::handleSigterm(sig_atomic_t signo)
{
    exit_signal = ENABLED;
}

void SignalHandler::handleSigusr1(sig_atomic_t signo)
{
    /* Wake up */
}
