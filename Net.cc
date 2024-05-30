#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>

#include <packet_m.h>
#include <packetLENGTH_m.h>

using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cOutVector hopCountVector;
    cOutVector sourceVector;
    cMessage *sendMsgEvent;
    int networkLength;

public:
    Net();
    virtual ~Net();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net()
{
    sendMsgEvent = NULL;
}

Net::~Net()
{
}

void Net::initialize()
{
    hopCountVector.setName("hopCount");
    sourceVector.setName("Source");

    sendMsgEvent = new cMessage("sendEvent");
    scheduleAt(simTime() + 0, sendMsgEvent);
}

void Net::finish()
{
}

void Net::handleMessage(cMessage *msg)
{
    // If this is a self message, we have to send a packet
    if (msg == sendMsgEvent)
    {
        PacketLENGTH *pktLENGTH = new PacketLENGTH("packetLENGTH", this->getParentModule()->getIndex());
        pktLENGTH->setByteLength(par(20));
        pktLENGTH->setSource(this->getParentModule()->getIndex());
        pktLENGTH->setHopCount(0);
        pktLENGTH->setKind(2);

        send(pktLENGTH, "toLnk$o");
        delete msg;
    }
    else if (msg->getKind() == 2)
    {
        PacketLENGTH *pktLENGTH = (PacketLENGTH *)msg;
        if (pktLENGTH->getSource() == this->getParentModule()->getIndex())
        {
            networkLength = pktLENGTH->getHopCount();
            delete pktLENGTH;
        }
        else
        {
            pktLENGTH->setHopCount(pktLENGTH->getHopCount() + 1);
            send(msg, "toLnk$o");
        }
    }
    else
    {
        Packet *pkt = (Packet *)msg;

        // If this node is the final destination, send to App
        if (pkt->getDestination() == this->getParentModule()->getIndex())
        {
            sourceVector.record(pkt->getSource());
            hopCountVector.record(pkt->getHopCount());
            send(msg, "toApp$o");
        }
        // If not, forward the packet to some else... to who?
        else
        {
            // We send to link interface #0, which is the
            // one connected to the clockwise side of the ring
            // Is this the best choice? are there others?
            pkt->setHopCount(pkt->getHopCount() + 1);
            send(msg, "toLnk$o", 0);
        }
    }
}
