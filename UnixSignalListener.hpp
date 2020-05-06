/**
 * Copyright (c) 2020 Anatolii Kurotych
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.


#pragma once

#include <functional>
#include <map>
#include <signal.h>

/*
 * Interface:
 *
 *  void init();
 *      "Block all UNIX signals. Should be called before creating any threads."
 *
 *  void setHandler(int signum, std::function<void()> handler, bool terminate = false);
 *      "Set function for handling UNIX signal."
 *      signum    — Unix signal
 *      handler   — Handler function
 *      terminate — Termination flag. If true, the listening will be stopped
 *                  and handling of signals will be unblocked.
 *
 * void listen();
 *      "Listen the unix signals. Blocks current thread.
 *      It can be unblocked if handler with flag terminate=true was triggered."
 *
 * void setTerminateSignal(int signum);
 *      "Set signal that terminates listening (without handler)"
 *
 * void setSigIgnore(int signum);
 *      "Set ignoring of a signal. Equals to: signal(signum, SIG_IGN);"
 *
 * void setTimeoutHandler(uint timeout, std::function<void()> handler);
 *      "Set timeout handler.
 *      The handler will be called by timeout if no signals are received."
 *
*/

class UnixSignalListener
{
public:
    using Handler = std::function<void()>;

    void init()
    {
        sigset_t mask;
        sigfillset(&mask);

        if (pthread_sigmask(SIG_SETMASK, &mask, nullptr) != 0)
        {
            throw std::runtime_error("Failed to block all signals");
        }

        mIsInited = true;
    }

    void setHandler(int signum, Handler handler, bool terminate = false)
    {
        if (sigaddset(&mSetHandlers, signum) != 0)
        {
            throw std::runtime_error("Not valid signal");
        }

        mHandlers[signum] = std::make_pair(handler, terminate);
    }

    void setTerminateSignal(int signum) { setHandler(signum, nullptr, true); }

    void setSigIgnore(int signum) { setHandler(signum, nullptr, false); }

    void setTimeoutHandler(uint timeout, Handler handler)
    {
        mTimeoutSpec.tv_sec = timeout;
        mTimeoutHandler = handler;
    }

    void listen()
    {
        checkRequirements();

        // Unblock all signals except set of handlers
        // This means that the handlers for unblocked signals will by default.
        if (pthread_sigmask(SIG_SETMASK, &mSetHandlers, nullptr) != 0)
        {
            throw std::runtime_error("Failed to set mask");
        }

        while (true)
        {
            int ret = sigtimedwait(&mSetHandlers, nullptr, &mTimeoutSpec);

            if (ret <= 0)
            {
                sigtimedwaitErrorHandler();
                continue;
            }

            if (signalProcessing(ret) == 1)
            {
                // stop listening
                break;
            }
        }
    }

    UnixSignalListener()
    {
        sigemptyset(&mSetHandlers);

        mTimeoutHandler = [] {/* Empty handler by default */};
    }

    ~UnixSignalListener() = default;
    UnixSignalListener(const UnixSignalListener &) = delete;
    UnixSignalListener &operator=(const UnixSignalListener &) = delete;

    UnixSignalListener(const UnixSignalListener &&) = delete;
    UnixSignalListener &operator=(const UnixSignalListener &&) = delete;

private:

    void sigtimedwaitErrorHandler()
    {
        if (errno == EAGAIN)
        {
            // No signal in timeout period
            mTimeoutHandler();
        }
        else if (errno == EINTR)
        {
            // received Unix signal that wasn't set in the mask
        }
        else if (errno == EINVAL)
        {
            throw std::runtime_error("Timeout invalid");
        }
    }

    int signalProcessing(int signum)
    {
        if (mHandlers.count(signum) != 1)
        {
            // "Internal error. Should be one handler
            throw std::runtime_error("We should never be here! Check the code");
        }

        if (mHandlers[signum].first != nullptr)
        {
            mHandlers[signum].first(); // Call handler
        }

        if (mHandlers[signum].second == true)
        {
            // It is terminating handler
            // Listening should be finished
            return 1;
        }

        return 0;
    }

    void checkRequirements()
    {
        if (!mIsInited)
        {
            throw std::runtime_error("UnixSignalListener was not initialized");
        }

        if (sigisemptyset(&mSetHandlers) == 1 // empty
            || mHandlers.empty())
        {
            throw std::runtime_error("There are no signal handlers to listen");
        }
    }

private:
    using HandlerPair = std::pair<Handler, bool>; // second parameter is "terminate" value

    std::map<int, HandlerPair> mHandlers; // Key is signum (number of unix signal)
    Handler mTimeoutHandler;
    sigset_t mSetHandlers;
    timespec mTimeoutSpec = { 600, 0 };   // Default timeout handler will be called once in ten mintes
    bool mIsInited = false;
};
