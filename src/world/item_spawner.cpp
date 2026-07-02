#include "world/item_spawner.hpp"

#include <chrono>

ItemSpawner::ItemSpawner(const Map& map)
    :map_(map),
    random_(
        static_cast<std::mt19937::result_type>(
            std::chrono::steady_clock::now()
                .time_since_epoch()
                .count()
        )
    )
{
}

void ItemSpawner::update(
    float deltaTime,
    std::vector<Item>& items,
    const std::vector<sf::FloatRect>& occupiedAreas
)
{
    if(deltaTime<=0.f)
        return;

    for(Item& item:items)
        item.updateAge(deltaTime);

    for(auto iterator=items.begin();
        iterator!=items.end();)
    {
        if(!iterator->isAlive()||
           iterator->isExpired(lifeTime_))
        {
            iterator=items.erase(iterator);
        }
        else
            ++iterator;
    }

    if(map_.empty()||
       map_.width()<=0||
       map_.height()<=0)
    {
        return;
    }

    updateSpawnTimer(
        damageSpawnTimer_,
        deltaTime,
        ItemType::DamagePack,
        items,
        occupiedAreas
    );

    updateSpawnTimer(
        healthSpawnTimer_,
        deltaTime,
        ItemType::HealthPack,
        items,
        occupiedAreas
    );
}

void ItemSpawner::updateSpawnTimer(
    float& timer,
    float deltaTime,
    ItemType type,
    std::vector<Item>& items,
    const std::vector<sf::FloatRect>& occupiedAreas
)
{
    timer-=deltaTime;

    if(timer>0.f)
        return;

    timer=spawnInterval_;

    if(hasAliveItemType(items,type))
        return;

    const sf::Vector2f position=
        findRandomSpawnPos(
            items,
            occupiedAreas
        );

    if(position.x<0.f||position.y<0.f)
        return;

    items.emplace_back(
        position,
        type
    );
}

bool ItemSpawner::hasAliveItemType(
    const std::vector<Item>& items,
    ItemType type
) const noexcept
{
    for(const Item& item:items)
    {
        if(item.isAlive()&&item.getType()==type)
            return true;
    }

    return false;
}

bool ItemSpawner::isValidSpawnPos(
    int x,
    int y,
    const std::vector<Item>& items,
    const std::vector<sf::FloatRect>& occupiedAreas
) const
{
    if(!map_.isInBounds(x,y))
        return false;

    const Tile* tile=map_.tileAt(x,y);

    if(tile==nullptr||
       tile->type()!=TileType::Empty)
    {
        return false;
    }

    const sf::Vector2f center=
        map_.tileCenter(x,y);

    constexpr float HalfItemSize=
        Item::DefaultSize*0.5f;

    const sf::FloatRect candidate
    {
        {
            center.x-HalfItemSize,
            center.y-HalfItemSize
        },
        {
            Item::DefaultSize,
            Item::DefaultSize
        }
    };

    for(const Item& item:items)
    {
        if(!item.isAlive())
            continue;

        if(candidate.findIntersection(
            item.getBounds()
        ).has_value())
        {
            return false;
        }
    }

    for(const sf::FloatRect& area:occupiedAreas)
    {
        if(candidate.findIntersection(area).has_value())
            return false;
    }

    return true;
}

sf::Vector2f ItemSpawner::findRandomSpawnPos(
    const std::vector<Item>& items,
    const std::vector<sf::FloatRect>& occupiedAreas
)
{
    if(map_.width()<=0||map_.height()<=0)
        return {-1.f,-1.f};

    std::uniform_int_distribution<int> xDistribution(
        0,
        map_.width()-1
    );

    std::uniform_int_distribution<int> yDistribution(
        0,
        map_.height()-1
    );

    constexpr int MaximumAttempts=100;

    for(int attempt=0;
        attempt<MaximumAttempts;
        ++attempt)
    {
        const int x=xDistribution(random_);
        const int y=yDistribution(random_);

        if(isValidSpawnPos(
            x,
            y,
            items,
            occupiedAreas
        ))
        {
            return map_.tileCenter(x,y);
        }
    }

    return {-1.f,-1.f};
}

