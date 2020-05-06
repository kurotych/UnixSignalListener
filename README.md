# C++ wrapper for processing UNIX signals. 
It is developed according to the common approach for multi-threaded programs. 

## Interface:
 
##### *Block all UNIX signals. Should be called before creating any threads.*
```void init()```  

---  
##### *Set function for handling UNIX signal.*
```void setHandler(int signum, std::function<void()> handler, bool terminate = false)```

---
##### *Listen the unix signals. Blocks current thread. It can be unblocked if handler with flag terminate=true was triggered.*
```void listen()```
 
---
##### *Set signal that terminates listening (without handler)*
```void setTerminateSignal(int signum)```

---
##### Set ignoring of a signal. Equals to: signal(signum, SIG_IGN);
```void setSigIgnore(int signum)```
 
##### Set timeout handler. The handler will be called by timeout if no signals are received.
```void setTimeoutHandler(uint timeout, std::function<void()> handler)```

---

Build and Run example:
```
g++ example.cpp -o unix_example -pthread
./unix_example
pkill unix_example (from other terminal)
```
