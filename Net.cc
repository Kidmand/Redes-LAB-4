#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>

#include <packet_m.h>

using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cOutVector hopCountVector;
    cOutVector sourceVector;
    cMessage *sendMsgEvent;
    int networkLength;
    bool readyToSend;
    cQueue buffer;
    // ------------------------------
    Packet *createPacketLENGTH();
    bool isMsgPacketLENGTH(cMessage *msg);
    bool isPacketForThisNode(Packet *pkt);
    void notifyReadyToSend();
    // ------------------------------

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

#define PKT_LENGTH_IDENTIFIER 2
#define PKT_LENGTH_ROUTE 0

Net::Net()
{
    sendMsgEvent = NULL;
    readyToSend = false;
    networkLength = 0;
}

Net::~Net()
{
    cancelAndDelete(sendMsgEvent);
}

void Net::initialize()
{
    // Se crea un mensaje para obtener la longitud de la red.
    Packet *pktLENGTH = createPacketLENGTH();
    send(pktLENGTH, "toLnk$o", 0);

    // Se crea un mensaje para enviar paquetes.
    sendMsgEvent = new cMessage("sendMsgEvent");

    // Se inicializan los vectores para gráficas:
    hopCountVector.setName("hopCount");
    sourceVector.setName("Source");
}

void Net::finish()
{
}

// ------------------- HANDLE MESSAGE ----------------------

void Net::handleMessage(cMessage *msg)
{
    if (msg == sendMsgEvent)
    {
        if (!buffer.isEmpty())
        {
            // dequeue
            Packet *pkt = (Packet *)buffer.pop();

            // send
            pkt->setHopCount(pkt->getHopCount() + 1);
            send(pkt, "toLnk$o", 0); // FIXME: se esta mandando todo por el mismo lado, luego arreglar usando algún protocolo.

            if (!sendMsgEvent->isScheduled())
            {
                // start the service now
                scheduleAt(simTime() + 0, sendMsgEvent);
            }
        }
    }
    else if (msg->arrivedOn("toApp$i"))
    {
        // enqueue
        buffer.insert(msg);

        if (!sendMsgEvent->isScheduled() && readyToSend)
        {
            // start the service now
            scheduleAt(simTime() + 0, sendMsgEvent);
        }
    }
    else
    {
        Packet *pkt = (Packet *)msg;

        if (isPacketForThisNode(pkt) && isMsgPacketLENGTH(msg))
        {
            networkLength = pkt->getHopCount();
            delete msg; // No necesitamos el paquete LENGTH.

            notifyReadyToSend();
        }
        else if (isPacketForThisNode(pkt) && !isMsgPacketLENGTH(msg))
        {
            sourceVector.record(pkt->getSource());
            hopCountVector.record(pkt->getHopCount());
            send(msg, "toApp$o");
        }
        else if (!isPacketForThisNode(pkt) && isMsgPacketLENGTH(msg))
        {
            pkt->setHopCount(pkt->getHopCount() + 1);
            send(msg, "toLnk$o", PKT_LENGTH_ROUTE);
        }
        else // !isPacketForThisNode(pkt) && !isPacketLENGTH(msg)
        {
            pkt->setHopCount(pkt->getHopCount() + 1);

            // Enviamos el paquete por el enlace contrario al que llegó.
            if (msg->arrivedOn("toLnk$0"))
            {
                send(msg, "toLnk$o", 1);
            }
            else
            {
                send(msg, "toLnk$o", 0);
            }
        }
    }
}

// ---------------- AUXILIARY FUNCTIONS -------------------

bool Net::isPacketForThisNode(Packet *pkt)
{
    return pkt->getDestination() == this->getParentModule()->getIndex();
}

bool Net::isMsgPacketLENGTH(cMessage *msg)
{
    Packet *pkt = (Packet *)msg;
    return msg->getKind() == PKT_LENGTH_IDENTIFIER && pkt->getDestination() == pkt->getSource();
}

Packet *Net::createPacketLENGTH()
{
    Packet *pktLENGTH = new Packet("packetLENGTH", PKT_LENGTH_IDENTIFIER);
    pktLENGTH->setByteLength(par("packetByteSizeLENGTH"));

    // Que el destino y el origen sean el mismo nodo es una invariante,
    // porque luego lo usamos para detectar este tipo de paquetes.
    pktLENGTH->setSource(this->getParentModule()->getIndex());
    pktLENGTH->setDestination(this->getParentModule()->getIndex());

    pktLENGTH->setHopCount(0);

    return pktLENGTH;
}

void Net::notifyReadyToSend()
{
    readyToSend = true;
    scheduleAt(simTime() + 0, sendMsgEvent);
}
