#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <string>
#include <vector>

struct DiscoveredHost
{
    std::string name;
    std::string address;
    unsigned short gamePort=0;
    float age=0.f;
};

class HostDiscovery
{
public:
    static constexpr unsigned short DiscoveryPort=54119;

    HostDiscovery();

    void beginAdvertising(unsigned short gamePort);
    void stopAdvertising();
    void beginListening();
    void stopListening();
    void update(float deltaTime);
    const std::vector<DiscoveredHost>& hosts() const noexcept;

private:
    void sendAnnouncement();
    void receiveAnnouncements();
    void addOrRefreshHost(const DiscoveredHost& host);

    sf::UdpSocket socket_;
    sf::Clock announceClock_;
    std::vector<DiscoveredHost> hosts_;
    bool advertising_=false;
    bool listening_=false;
    unsigned short gamePort_=0;
};
