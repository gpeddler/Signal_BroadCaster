//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "QueueExt.h"
#include "Job.h"

Define_Module(QueueExt);

void QueueExt::arrival(queueing::Job *job)
{
    Queue::arrival(job);
    preL++;
}

simtime_t QueueExt::startService(queueing::Job *job)
{
    preLq = length();
    return Queue::startService(job);
}

void QueueExt::endService(queueing::Job *job)
{
    //Queue::endService(job);
    simtime_t d = simTime() - job->getTimestamp();
    job->setTotalServiceTime(job->getTotalServiceTime() + d);
    send(job, "out", 1);

    qWqstat.collect2(length(), simTime() - job->getTimestamp() );
    preLq = length();
    preL--;
}

void QueueExt::initialize()
{
    Queue::initialize();

    name = par("name");
    preL = preLq = 0;
    preTime = simTime();
    signalReceived = false;

    /*
    char fname[50] = {0,};
    if(ev.getConfigEx()->getActiveRunNumber() == 0)
    {
        sprintf(fname, "curveL%d.txt", name);
        outL = fopen(fname, "w");
        sprintf(fname, "curveLq%d.txt", name);
        outLq = fopen(fname, "w");
        sprintf(fname, "curveWq%d.txt", name);
        outWq = fopen(fname, "w");
        sprintf(fname, "curveU%d.txt", name);
        outU = fopen(fname, "w");


        fprintf(outL, "0.00 0.00\n");
        fprintf(outLq, "0.00 0.00\n");
        fprintf(outWq, "0.00 0.00\n");
        fprintf(outU, "0.00 0.00\n");
    }
    else
    {
        sprintf(fname, "curveL%d.txt", name);
        outL = fopen(fname, "a");
        sprintf(fname, "curveLq%d.txt", name);
        outLq = fopen(fname, "a");
        sprintf(fname, "curveWq%d.txt", name);
        outWq = fopen(fname, "a");
        sprintf(fname, "curveU%d.txt", name);
        outU = fopen(fname, "a");
    }
    */

    qLstat.setName("# of jobs in the system");
    qLqstat.setName("# of jobs in the queue");
    qWqstat.setName("queueing delay");
/*
    stats.setRangeAutoUpper(0,10,1.5);
*/
}

void QueueExt::handleMessage(cMessage *msg)
{
    // EV << msg->getSenderModuleId() << endl;

    /* When the message is received from signal processor */
    if ( msg->getSenderModuleId() == 2 )
    {
        if ( msg->getKind() == 0 ) signalReceived = false;
        else signalReceived = true;
        delete msg;
        return;
    }

    if ( signalReceived )
    {
        Queue::handleMessage(msg);
    }
    else
    {
        if ( msg == endServiceMsg ) scheduleAt( simTime()+1, msg );
        else
        {
            queueing::Job *job = check_and_cast<queueing::Job *>(msg);
            arrival(job);
            queue.insert( job );
            emit(queueLengthSignal, length());
            job->setQueueCount(job->getQueueCount() + 1);
        }

    }

    qLstat.collect2(preL, simTime() - preTime );
    qLqstat.collect2(preLq, simTime() - preTime );
    if( length() > 0 ) busyTime += simTime() - preTime;

    preTime = simTime();
}

void QueueExt::finish()
{
//  EV << "Count" << ev.getConfigEx()->getActiveRunNumber() << endl;
    int totalSimulTime = 10000;
    if( length() > 0 ) busyTime += simTime() - preTime;
    qUstat.collect( busyTime );

    while( length() > 0 )
    {
        queueing::Job *j = getFromQueue();
        qWqstat.collect2(length(), simTime() - j->getTimestamp() );
    }

    EV << "name " << name << endl;
    EV << "meanL " << qLstat.getMean();
    EV << " meanLq " << qLqstat.getMean();
    EV << " meanWq " << qWqstat.getMean() << endl;
    EV << "Util : " << qUstat.getMax() / totalSimulTime << endl;

    /*
    fprintf(outL, "%.3f %f\n", (double)(ev.getConfigEx()->getActiveRunNumber())*(double)0.005 + 0.05, qLstat.getMean() );
    fprintf(outLq, "%.3f %f\n", (double)(ev.getConfigEx()->getActiveRunNumber())*(double)0.005 + 0.05, qLqstat.getMean() );
    fprintf(outWq, "%.3f %f\n", (double)(ev.getConfigEx()->getActiveRunNumber())*(double)0.005 + 0.05, qWqstat.getMean() );
    fprintf(outU, "%.3f %f\n", (double)(ev.getConfigEx()->getActiveRunNumber())*(double)0.005 + 0.05, qUstat.getMax()/totalSimulTime );

    fclose(outL);
    fclose(outLq);
    fclose(outWq);
    fclose(outU);
    */
}
