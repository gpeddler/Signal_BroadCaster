//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __SINKEXT_H__
#define __SINKEXT_H__

#include <omnetpp.h>
#include "Queue.h"
#include "Job.h"

class QueueExt : public queueing::Queue
{
private:
    cWeightedStdDev qLstat, qLqstat, qWqstat;
    cStdDev qUstat;
    simtime_t preTime, busyTime;
    int preL, preLq;
    int name;
    bool signalReceived;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void arrival(queueing::Job *job);
    virtual simtime_t startService(queueing::Job *job);
    virtual void endService(queueing::Job *job);
};

#endif
