#include "network/network_session.hpp"

#include <SFML/System.hpp>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace
{
bool expect(bool condition,const std::string& message)
{
    if(condition)
    {
        std::cout<<"[PASS] "<<message<<"\n";
        return true;
    }

    std::cout<<"[FAIL] "<<message<<"\n";
    return false;
}

void pump(NetworkSession& host,NetworkSession& client,float seconds)
{
    sf::Clock clock;
    while(clock.getElapsedTime().asSeconds()<seconds)
    {
        host.update();
        client.update();
        sf::sleep(sf::milliseconds(10));
    }
}

GameSnapshot makeSnapshot()
{
    GameSnapshot snapshot;
    snapshot.status=1;
    snapshot.baseHp=80;
    snapshot.baseAlive=true;

    snapshot.playerOne.id=1;
    snapshot.playerOne.x=120.f;
    snapshot.playerOne.y=160.f;
    snapshot.playerOne.direction=2;
    snapshot.playerOne.hp=90;
    snapshot.playerOne.alive=true;

    TankSnapshot enemy;
    enemy.id=10;
    enemy.x=300.f;
    enemy.y=200.f;
    enemy.direction=1;
    enemy.hp=50;
    enemy.alive=true;
    snapshot.enemies.push_back(enemy);

    BulletSnapshot bullet;
    bullet.x=180.f;
    bullet.y=220.f;
    bullet.faction=0;
    bullet.damage=20;
    bullet.ownerId=1;
    bullet.alive=true;
    bullet.dirX=0.f;
    bullet.dirY=-1.f;
    snapshot.bullets.push_back(bullet);

    ItemSnapshot item;
    item.x=240.f;
    item.y=280.f;
    item.type=1;
    item.alive=true;
    snapshot.items.push_back(item);

    snapshot.destroyedBricks.emplace_back(3,4);
    return snapshot;
}
}

int main()
{
    bool ok=true;

    NetworkSession host;
    NetworkSession client;
    constexpr unsigned short Port=54118;

    ok=expect(host.startHost(Port),"host starts listening")&&ok;

    std::atomic_bool accepting=true;
    std::thread acceptThread(
        [&host,&accepting]()
        {
            while(accepting)
            {
                host.update();
                sf::sleep(sf::milliseconds(10));
            }
        }
    );

    ok=expect(client.connectToHost("127.0.0.1",Port),"client connects to host")&&ok;

    sf::sleep(sf::milliseconds(200));
    accepting=false;
    acceptThread.join();

    pump(host,client,0.5f);

    ok=expect(host.isConnected(),"host reaches connected state")&&ok;
    ok=expect(client.isConnected(),"client reaches connected state")&&ok;

    bool hostReceivedHello=false;
    bool clientReceivedWelcome=false;
    for(int i=0;i<20;++i)
    {
        pump(host,client,0.05f);
        hostReceivedHello=host.pollWelcome()||hostReceivedHello;
        clientReceivedWelcome=client.pollWelcome()||clientReceivedWelcome;
    }

    ok=expect(hostReceivedHello,"host receives client hello")&&ok;
    ok=expect(clientReceivedWelcome,"client receives host welcome")&&ok;

    PlayerInputState sentInput;
    sentInput.moveRight=true;
    sentInput.fire=true;
    ok=expect(client.sendInput(sentInput),"client sends input")&&ok;

    PlayerInputState receivedInput;
    bool inputReceived=false;
    for(int i=0;i<20&&!inputReceived;++i)
    {
        pump(host,client,0.05f);
        inputReceived=host.pollRemoteInput(receivedInput);
    }

    ok=expect(inputReceived,"host polls remote input")&&ok;
    ok=expect(receivedInput.moveRight&&receivedInput.fire,"remote input payload matches")&&ok;

    const GameSnapshot sentSnapshot=makeSnapshot();
    ok=expect(host.sendSnapshot(sentSnapshot),"host sends snapshot")&&ok;

    GameSnapshot receivedSnapshot;
    bool snapshotReceived=false;
    for(int i=0;i<20&&!snapshotReceived;++i)
    {
        pump(host,client,0.05f);
        snapshotReceived=client.pollSnapshot(receivedSnapshot);
    }

    ok=expect(snapshotReceived,"client polls snapshot")&&ok;
    ok=expect(receivedSnapshot.baseHp==80,"snapshot base hp matches")&&ok;
    ok=expect(receivedSnapshot.enemies.size()==1,"snapshot enemy count matches")&&ok;
    ok=expect(receivedSnapshot.items.size()==1,"snapshot item count matches")&&ok;
    ok=expect(receivedSnapshot.destroyedBricks.size()==1,"snapshot brick change count matches")&&ok;

    ok=expect(host.sendTileChanged(6,7,0),"host sends tile change")&&ok;

    int tileX=0;
    int tileY=0;
    int tileType=-1;
    bool tileReceived=false;
    for(int i=0;i<20&&!tileReceived;++i)
    {
        pump(host,client,0.05f);
        tileReceived=client.pollTileChanged(tileX,tileY,tileType);
    }

    ok=expect(tileReceived,"client polls tile change")&&ok;
    ok=expect(tileX==6&&tileY==7&&tileType==0,"tile change payload matches")&&ok;

    host.disconnect();
    client.disconnect();

    if(!ok)
        return 1;

    std::cout<<"network_test passed\n";
    return 0;
}
