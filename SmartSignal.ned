
import org.omnetpp.queueing.Queue;
import org.omnetpp.queueing.Sink;
import org.omnetpp.queueing.Source;

network SmartSignal
{
    parameters:
        @display("i=block/network2;bgb=642,503");
    submodules:
        signal: CpuQueueExt {
            @display("p=329,232");
        }
        top: QueueExt {
            @display("p=302,27");
        }
        left: QueueExt {
            @display("p=33,204");
        }
        right: QueueExt {
            @display("p=603,261");
        }
        bottom: QueueExt {
            @display("p=359,467");
        }
        topSource: Source {
            @display("p=251,27");
        }
        leftSource: Source {
            @display("p=33,143");
        }
        rightSource: Source {
            @display("p=603,331");
        }
        bottomSource: Source {
            @display("p=435,467");
        }
        topSink: SinkExt {
            @display("p=302,467");
        }
        leftSink: SinkExt {
            @display("p=603,203");
        }
        rightSink: SinkExt {
            @display("p=33,262");
        }
        bottomSink: SinkExt {
            @display("p=359,27");
        }

    connections:
        topSource.out --> top.in++;
        leftSource.out --> left.in++;
        rightSource.out --> right.in++;
        bottomSource.out --> bottom.in++;
        top.out++ --> signal.in++;
        top.out++ --> topSink.in++;
        left.out++ --> signal.in++;
        left.out++ --> leftSink.in++;
        right.out++ --> signal.in++;
        right.out++ --> rightSink.in++;
        bottom.out++ --> signal.in++;
        bottom.out++ --> bottomSink.in++;
        signal.out++ --> top.in++;
        signal.out++ --> left.in++;
        signal.out++ --> right.in++;
        signal.out++ --> bottom.in++;

}
