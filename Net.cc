#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>

#include <packet_m.h>
#include <packetLENGTH_m.h>
#include <packetREADY_m.h>

using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cOutVector hopCountVector;
    cOutVector sourceVector;
    cMessage *pktLengthEvent;
    int networkLength;
    int pktLengthIdentifier;
    int pktReadyIdentifier;

public:
    Net();
    virtual ~Net();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

private:
    PacketLENGTH *createPacketLENGTH();
    PacketREADY *createPacketREADY();
    bool isPacketLENGTH(cMessage *msg);
    void notifyAppSendPkts();
};

Define_Module(Net);

#endif /* NET */

Net::Net()
{
    pktLengthEvent = NULL;
    networkLength = 0;
    pktLengthIdentifier = 255;
    pktReadyIdentifier = 256; // FIMXE: no me gusta esto y que ademas se use en app.cc
}

Net::~Net()
{
    cancelAndDelete(pktLengthEvent);
}

void Net::initialize()
{
    // Se crea un evento para recolectar la longitud de la red:
    pktLengthEvent = new cMessage("pktLENGTHEvent");
    scheduleAt(simTime() + 0, pktLengthEvent);

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
    // Recibe un mensaje de tipo PacketLENGTH
    if (msg == pktLengthEvent)
    {
        PacketLENGTH *pktLENGTH = createPacketLENGTH();
        send(pktLENGTH, "toLnk$o", 0);
        delete msg; // Se elimina el mensaje porque no se usa más.
    }
    else if (isPacketLENGTH(msg))
    {
        PacketLENGTH *pktLENGTH = (PacketLENGTH *)msg;

        // Si el mensaje es de origen, se guarda la longitud de la red.
        if (pktLENGTH->getSource() == this->getParentModule()->getIndex())
        {
            networkLength = pktLENGTH->getHopCount();
            delete pktLENGTH;

            // Se envía un mensaje a la aplicación para notificar que la red está lista.
            notifyAppSendPkts();
        }
        // Si el mensaje no es de origen, se reenvía a otro nodo y se incrementa el contador de saltos.
        else
        {
            pktLENGTH->setHopCount(pktLENGTH->getHopCount() + 1);
            send(msg, "toLnk$o", 0);
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

// ---------------- AUXILIARY FUNCTIONS -------------------

bool Net::isPacketLENGTH(cMessage *msg)
{
    return msg->getKind() == pktLengthIdentifier;
}

PacketLENGTH *Net::createPacketLENGTH()
{
    PacketLENGTH *pktLENGTH = new PacketLENGTH("packetLENGTH", this->getParentModule()->getIndex());
    pktLENGTH->setByteLength(par("packetByteSizeLENGTH"));
    pktLENGTH->setSource(this->getParentModule()->getIndex());
    pktLENGTH->setHopCount(0);
    pktLENGTH->setKind(pktLengthIdentifier);

    return pktLENGTH;
}

PacketREADY *Net::createPacketREADY()
{
    PacketREADY *pktREADY = new PacketREADY("packetREADY", this->getParentModule()->getIndex());
    pktREADY->setByteLength(par("packetByteSizeREADY"));
    pktREADY->setIsNetworkReady(true);
    pktREADY->setKind(pktReadyIdentifier);

    return pktREADY;
}

void Net::notifyAppSendPkts()
{
    PacketREADY *pktREADY = createPacketREADY();
    send(pktREADY, "toApp$o");
}
