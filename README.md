# C++ wrapper for processing UNIX signals. 
It is developed according to the common approach for multi-threaded programs. 

## Interface:
 
```void init()```  
##### *Block all UNIX signals. Should be called before creating any threads.*
 
```void setHandler(int signum, std::function<void()> handler, bool terminate = false)```
##### *Set function for handling UNIX signal.*

```void listen()```
##### *Listen the unix signals. Blocks current thread. It can be unblocked if handler with flag terminate=true was triggered.*
 
```void setTerminateSignal(int signum)```
##### *Set signal that terminates listening (without handler)*
 
```void setSigIgnore(int signum)```
##### Set ignoring of a signal. Equals to: signal(signum, SIG_IGN);
 
```void setTimeoutHandler(uint timeout, std::function<void()> handler)```
##### Set timeout handler. The handler will be called by timeout if no signals are received.
 
Build and Run example:
```
g++ example.cpp -o unix_example -pthread
./unix_example
pkill unix_example (from other terminal)
```
