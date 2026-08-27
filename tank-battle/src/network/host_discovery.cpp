#include "network/host_discovery.hpp"

#include <algorithm>
#include <optional>
#include <string>

namespace
{
constexpr const char* DiscoveryMagic="TankBattleHost";
constexpr float AnnouncementInterval=1.f;
constexpr float HostTimeout=4.f;

std::optional<sf::IpAddress> localBroadcastAddress()
{
    const std::optional<sf::IpAddress> localAddress=sf::IpAddress::getLocalAddress();
    if(!localAddress.has_value())
        return std::nullopt;

    const std::string address=localAddress->toString();
    const std::size_t lastDot=address.rfind('.');
    if(lastDot==std::string::npos)
        return std::nullopt;

    return sf::IpAddress::fromString(address.substr(0,lastDot+1)+"255");
}

sf::Packet makeAnnouncementPacket(unsigned short gamePort)
{
    sf::Packet packet;
    packet<<std::string(DiscoveryMagic)<<std::string("Tank Battle Host")<<gamePort;
    return packet;
}
}

HostDiscovery::HostDiscovery()
{
    socket_.setBlocking(false);
}

void HostDiscovery::beginAdvertising(unsigned short gamePort)
{
    if(listening_)
        stopListening();

    socket_.unbind();
    if(socket_.bind(sf::Socket::AnyPort)!=sf::Socket::Status::Done)
        return;

    advertising_=true;
    gamePort_=gamePort;
    socket_.setBlocking(false);
    announceClock_.restart();
    sendAnnouncement();
}

void HostDiscovery::stopAdvertising()
{
    advertising_=false;
    socket_.unbind();
}

void HostDiscovery::beginListening()
{
    if(advertising_)
        stopAdvertising();

    if(listening_)
        return;

    socket_.unbind();
    if(socket_.bind(DiscoveryPort)!=sf::Socket::Status::Done)
        return;

    socket_.setBlocking(false);
    hosts_.clear();
    listening_=true;
}

void HostDiscovery::stopListening()
{
    if(!listening_)
        return;

    listening_=false;
    hosts_.clear();
    socket_.unbind();
}

void HostDiscovery::update(float deltaTime)
{
    if(advertising_ && announceClock_.getElapsedTime().asSeconds()>=AnnouncementInterval)
    {
        sendAnnouncement();
        announceClock_.restart();
    }

    if(listening_)
        receiveAnnouncements();

    for(auto& host:hosts_)
        host.age+=deltaTime;

    hosts_.erase(
        std::remove_if(
            hosts_.begin(),
            hosts_.end(),
            [](const DiscoveredHost& host)
            {
                return host.age>HostTimeout;
            }
        ),
        hosts_.end()
    );
}

const std::vector<DiscoveredHost>& HostDiscovery::hosts() const noexcept{return hosts_;}

void HostDiscovery::sendAnnouncement()
{
    sf::Packet broadcastPacket=makeAnnouncementPacket(gamePort_);
    sf::Socket::Status status=socket_.send(broadcastPacket,sf::IpAddress::Broadcast,DiscoveryPort);
    (void)status;

    const std::optional<sf::IpAddress> subnetBroadcast=localBroadcastAddress();
    if(subnetBroadcast.has_value() && subnetBroadcast.value()!=sf::IpAddress::Broadcast)
    {
        sf::Packet subnetPacket=makeAnnouncementPacket(gamePort_);
        status=socket_.send(subnetPacket,subnetBroadcast.value(),DiscoveryPort);
        (void)status;
    }
}

void HostDiscovery::receiveAnnouncements()
{
    sf::Packet packet;
    std::optional<sf::IpAddress> remoteAddress;
    unsigned short remotePort=0;

    while(socket_.receive(packet,remoteAddress,remotePort)==sf::Socket::Status::Done)
    {
        std::string magic;
        std::string name;
        unsigned short gamePort=0;

        if((packet>>magic>>name>>gamePort) && magic==DiscoveryMagic && remoteAddress.has_value())
        {
            addOrRefreshHost(
                {
                    name,
                    remoteAddress->toString(),
                    gamePort,
                    0.f
                }
            );
        }

        packet.clear();
    }
}

void HostDiscovery::addOrRefreshHost(const DiscoveredHost& host)
{
    for(auto& current:hosts_)
    {
        if(current.address==host.address && current.gamePort==host.gamePort)
        {
            current.name=host.name;
            current.age=0.f;
            return;
        }
    }

    hosts_.push_back(host);
}
