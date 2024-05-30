#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>

#include <packet_m.h>
#include <packetNETWORK_m.h>

using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cOutVector hopCountVector;
    cOutVector sourceVector;
    cMessage *sendPktAppEvent;
    int networkLength;
    bool readyToSend;
    cQueue buffer;
    int *networkArray;
    // ------------------------------
    Packet *createPacketLENGTH();
    PacketNETWORK *createPacketNETWORK();
    bool isMsgPacketLENGTH(cMessage *msg);
    bool isMsgPacketNETWORK(cMessage *msg);
    bool isPacketForThisNode(Packet *pkt);
    bool isPacketForThisNode(PacketNETWORK *pkt);
    void sendToOppositeLnk(cMessage *msg);
    void notifyReadyToSend();
    int routePacket(Packet *pkt);
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

#define PKT_NETWORK_IDENTIFIER 1
#define PKT_LENGTH_IDENTIFIER 2
#define PKT_LENGTH_ROUTE 0
#define PKT_NETWORK_ROUTE 0

Net::Net()
{
    sendPktAppEvent = NULL;
    readyToSend = false;
    networkLength = 0;
    networkArray = NULL;
}

Net::~Net()
{
    cancelAndDelete(sendPktAppEvent);
    delete[] networkArray;
}

void Net::initialize()
{
    // Se crea un mensaje para obtener la longitud de la red.
    Packet *pktLENGTH = createPacketLENGTH();
    send(pktLENGTH, "toLnk$o", 0);

    // Se crea un mensaje para enviar paquetes.
    sendPktAppEvent = new cMessage("sendPktAppEvent");

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
    if (msg == sendPktAppEvent)
    {
        if (!buffer.isEmpty())
        {
            // dequeue
            Packet *pkt = (Packet *)buffer.pop();

            pkt->setHopCount(pkt->getHopCount() + 1);

            // send
            int route = routePacket(pkt);
            send(pkt, "toLnk$o", route);

            if (!sendPktAppEvent->isScheduled())
            {
                // start the service now
                scheduleAt(simTime() + 0, sendPktAppEvent);
            }
        }
    }
    else if (msg->arrivedOn("toApp$i"))
    {
        // enqueue
        buffer.insert(msg);

        if (!sendPktAppEvent->isScheduled() && readyToSend)
        {
            // start the service now
            scheduleAt(simTime() + 0, sendPktAppEvent);
        }
    }
    else // Si el mensaje vine por algún enlace (osea toLnk$0 || toLnk$1).
    {
        // NOTE: Para leer el código, colapsar los bloques de if y else.
        //       Luego abrir solo el bloque que se quiere leer.

        Packet *pkt = (Packet *)msg;

        // Si el paquete SI es para este nodo y NO es de control, lo enviamos a la aplicación.
        if (isPacketForThisNode(pkt) && !isMsgPacketLENGTH(msg) && !isMsgPacketNETWORK(msg))
        {
            sourceVector.record(pkt->getSource());
            hopCountVector.record(pkt->getHopCount());
            send(msg, "toApp$o");
        }
        // Si el paquete SI es para este nodo y SI es de control, lo procesamos.
        else if (isPacketForThisNode(pkt) && isMsgPacketNETWORK(msg))
        {
            // Creamos el arreglo de que representa la red.
            networkArray = new int[networkLength];

            // Le damos valores al arreglo de nodos.
            PacketNETWORK *pktNETWORK = (PacketNETWORK *)msg;

            for (int i = 0; i < networkLength; ++i)
            {
                networkArray[i] = pktNETWORK->getNetwork(i);
            }

            delete pktNETWORK; // No necesitamos el paquete NETWORK.

            // Notificamos que estamos listos para enviar paquetes.
            notifyReadyToSend();
        }
        else if (isPacketForThisNode(pkt) && isMsgPacketLENGTH(msg))
        {
            networkLength = pkt->getHopCount();
            delete msg; // No necesitamos el paquete LENGTH.

            // Enviamos un paquete NETWORK, para conocer la red.
            PacketNETWORK *pktNETWORK = createPacketNETWORK();
            send(pktNETWORK, "toLnk$o", PKT_NETWORK_ROUTE);
        }
        // Si el paquete NO es para este nodo y SI es de control, lo procesamos.
        else if (!isPacketForThisNode(pkt) && isMsgPacketNETWORK(msg))
        {
            // Agregamos nuestro nombre al arreglo de nodos.
            PacketNETWORK *pktNETWORK = (PacketNETWORK *)msg;
            int indice = pktNETWORK->getIndice();
            pktNETWORK->setNetwork(indice, (this->getParentModule()->getIndex()));

            // Reducimos la longitud de la red, porque se utiliza como índice.
            pktNETWORK->setIndice(indice + 1);

            // Enviamos el paquete por el enlace contrario al que llegó.
            sendToOppositeLnk(msg);
        }
        else if (!isPacketForThisNode(pkt) && isMsgPacketLENGTH(msg))
        {
            pkt->setHopCount(pkt->getHopCount() + 1);
            send(msg, "toLnk$o", PKT_LENGTH_ROUTE);
        }
        // Si el paquete NO es para este nodo y NO es de control, lo enviamos a la red SIN PROCESAR.
        else // !isPacketForThisNode(pkt) && !isPacketLENGTH(msg) && !isPacketNETWORK(msg)
        {
            pkt->setHopCount(pkt->getHopCount() + 1);

            // Enviamos el paquete por el enlace contrario al que llegó.
            sendToOppositeLnk(msg);
        }

        // NOTE: Tenemos 3 variables => (2^3 = 8) casos posibles para manejar.
        //       En el código se contemplan 6 casos, los 2 restantes son los que no se contemplan.

        // NOTE: El caso: " isMsgPacketLENGTH(msg) && isMsgPacketNETWORK(msg) " no se debería dar nunca.
        //       Con isPacketForThisNode(pkt) = {true, false} se forman
        //       los 2 casos que no se contemplan en el código.
    }
}

// ---------------- FUNCTIONS FOR ANY PACKET ----------------

bool Net::isPacketForThisNode(Packet *pkt)
{
    return pkt->getDestination() == this->getParentModule()->getIndex();
}

bool Net::isPacketForThisNode(PacketNETWORK *pkt)
{
    return pkt->getDestination() == this->getParentModule()->getIndex();
}

// ------------- FUNCTIONS FOR PACKET LENGTH -----------------

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

// -------------- FUNCTIONS FOR PACKET NETWORK ----------------

bool Net::isMsgPacketNETWORK(cMessage *msg)
{
    PacketNETWORK *pktNETWORK = (PacketNETWORK *)msg;
    return msg->getKind() == PKT_NETWORK_IDENTIFIER && pktNETWORK->getDestination() == pktNETWORK->getSource();
}

PacketNETWORK *Net::createPacketNETWORK()
{
    PacketNETWORK *pktNETWORK = new PacketNETWORK("packetNETWORK", PKT_NETWORK_IDENTIFIER);
    pktNETWORK->setByteLength(par("packetByteSizeNETWORK").intValue() + networkLength * 4);

    pktNETWORK->setSource(this->getParentModule()->getIndex());
    pktNETWORK->setDestination(this->getParentModule()->getIndex());
    pktNETWORK->setIndice(0);

    // Inicializamos el arreglo de nodos.
    for (int i = 0; i < networkLength + 1; ++i)
    {
        pktNETWORK->appendNetwork(-1);
    }

    return pktNETWORK;
}

// ----------------- FUNCTIONS FOR SENDING -------------------

void Net::sendToOppositeLnk(cMessage *msg)
{
    if (msg->arrivedOn("toLnk$i", 0))
    {
        send(msg, "toLnk$o", 1);
    }
    else
    {
        send(msg, "toLnk$o", 0);
    }
}

void Net::notifyReadyToSend()
{
    readyToSend = true;
    scheduleAt(simTime() + 0, sendPktAppEvent);
}

// ----------------- FUNCTIONS FOR ROUTING -------------------

// Para usarla tiene que esta definido el arreglo networkArray y la longitud networkLength.
int Net::routePacket(Packet *pkt)
{
    int dest = pkt->getDestination();
    bool found = false;

    for (size_t i = 0; i < networkLength / 2; i++)
    {
        if (networkArray[i] == dest)
        {

            found = true;
            break;
        }
    }
    EV << "]" << endl;

    if (found)
    {
        return PKT_NETWORK_ROUTE;
    }
    else
    {
        return PKT_NETWORK_ROUTE == 0 ? 1 : 0;
    }
}