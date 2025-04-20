#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <algorithm>
#include <functional>
#include <ctime>
#include <random>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <iomanip>
using namespace std;
using namespace chrono;

class Player;
class Enemy;
class Item;
class StatusEffect;
class Skill;
class Inventory;

// RANDOMIZER random
class RandomGenerator
{
    mt19937 mt;

public:
    RandomGenerator() : mt(time(0)) {}

    int getRandom(int min, int max)
    {
        uniform_int_distribution<int> dist(min, max);
        return dist(mt);
    }

    bool chance(int p) { return getRandom(1, 100) <= p; }

    bool critical() { return chance(15); }
};

RandomGenerator gen;

// LOGGER: TO KEEP RECORD OF EVENTS HAPPENING IN THE GAME!
//  iostream , vector , string , ctime , sstream , algoroithm
class Logger
{
private:
    vector<string> events;
    static const int max = 100;

public:
    void logEvent(const string &event)
    {
        string timeStamp = currentTimeStamp();
        events.push_back("[" + timeStamp + "]" + ": " + event);

        if (events.size() > max)
        {
            events.erase(events.begin());
        }
        cout << endl;
        cout << "LOG: " << event << endl
             << endl;
    }

    void logItemUse(const Item *item)
    {
        string itemName;

        if (item)
        {
            itemName = item->getName(); // getmname MUST inlcude in item!!!
        }
        else
        {
            itemName = "UNKNOWN ITEM";
        }

        logEvent("Used item: " + itemName);
    }

    vector<string> getRecentLogs(int count)
    {
        count = min(count, static_cast<int>(events.size()));
        return vector<string>(events.end() - count, events.end());
    }

    void display()
    {
        cout << "--------EVENTS-------" << endl
             << endl;
        for (const auto &log : events)
        {
            cout << log << endl
                 << endl;
        }
        cout << endl
             << endl;
    }

    void logBattle(const Player &player, const Enemy &enemy); // MUST DEFINE AFTER PLAYER AND ENEMY

private:
    string currentTimeStamp()
    {
        auto now = system_clock::now();           // get time_point current
        auto time = system_clock::to_time_t(now); // human-readbale form

        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

Logger gameLogger;

// HEART SYSTEM USING DATA STRUCTURES(LINKED LIST)

class Heart
{
public:
    bool isFull;
    Heart *next;
    Heart() : isFull(true), next(nullptr) {}
};

class HeartSystem
{
    Heart *head;
    int maxHearts;
    int currHearts;

public:
    HeartSystem(int cnt = 5) : head(nullptr), maxHearts(cnt), currHearts(0)
    {
        for (int i = 0; i < cnt; ++i)
        {
            addHeart();
        }
    }
    ~HeartSystem()
    {
        Heart *current = head;
        while (current != nullptr)
        {
            Heart *next = current->next;
            delete current;
            current = next;
        }
    }

    void addHeart()
    {
        // TO CHECK IF LIMIT REACHDE
        if (currHearts >= maxHearts)
            return;

        Heart *newHeart = new Heart();

        if (head == nullptr)
        {
            head = newHeart;
        }
        else
        {
            Heart *current = head;
            // FIDN THE LAST HEART
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = newHeart;
        }

        currHearts++;
    }

    bool loseHeart()
    {
        // if no HEARTS ARE LEFT
        if (head == nullptr || currHearts <= 0)
        {
            return false;
        }

        // TO FIND THE LAST FULL HEART
        Heart *current = head;
        Heart *lastFull = nullptr;

        while (current != nullptr)
        {
            if (current->isFull)
            {
                lastFull = current;
            }
            current = current->next;
        }

        if (lastFull != nullptr)
        {
            lastFull->isFull = false;
            currHearts--;
            return true;
        }

        return false;
    }

    bool gainHeart()
    {
        // IF HEARTS ARE ALREAADY MNAXED OUT
        if (currHearts >= maxHearts)
        {
            return false;
        }

        // FIDN THE FIRST EMPTY HEART
        Heart *current = head;
        while (current != nullptr && current->isFull)
        {
            current = current->next;
        }

        if (current != nullptr)
        {
            current->isFull = true;
            currHearts++;
            return true;
        }

        return false;
    }

    int getCurrHearts() const { return currHearts; }
    int getMaxHearts() const { return maxHearts; }

    void display() const
    {
        cout << "Hearts: " << endl;
        Heart *current = head;
        while (current != nullptr)
        {
            cout << (current->isFull ? "FULL" : "EMPTY") << " ";
            current = current->next;
        }
        cout << "[ " << currHearts << " / " << maxHearts << " ] " << endl
             << endl;
    }
};

// CUSTOM STACK(DATA STRUCTURE) FOR LEVELS IGNAME!

class LevelNode
{
public:
    int level;
    int requiredXP;
    LevelNode *previous;
    LevelNode(int lvl, int reqXP) : level(lvl), requiredXP(reqXP), previous(nullptr) {}
};

class Stack
{
    LevelNode *top;

public:
    Stack() : top(nullptr)
    {
        for (int i = 10; i >= 1; --i)
        {
            push(i, i * i * 100);
        }
    }

    ~Stack()
    {
        while (top != nullptr)
        {
            pop();
        }
    }

    void push(int level, int requiredXP)
    {
        LevelNode *newNode = new LevelNode(level, requiredXP);
        newNode->previous = top;
        top = newNode;
    }

    LevelNode *pop()
    {
        if (isEmpty())
        {
            return nullptr;
        }
        LevelNode *temp = top;
        top = top->previous;
        temp->previous = nullptr;
        return temp;
    }

    bool isEmpty() const { return top == nullptr; }
    LevelNode *peek() const { return top; }
    int getCurrLevel() const { return (isEmpty() ? 0 : top->level); }
    int getreqXPForNextLevel() const { return (isEmpty() ? 100 : top->requiredXP); }

    LevelNode *levelUp()
    {
        return pop();
    }
};

// EFFECTS!!

class Effect
{
protected:
    string name;
    int duration;
    string description;

public:
    Effect(const string &n, int dur, const string &desc) : name(n), duration(dur), description(desc) {}
    virtual ~Effect() = default;
    virtual void apply(Player &player) = 0; /// MUST APPLY ONCE PLAYER CLASS IS MAED!!
    void Decrementduration()
    {
        if (duration > 0)
        {
            duration--;
        }
    }

    bool isExpired() const { return duration <= 0; }

    const string &getName() const
    {
        return name;
    }

    const string &getDescription() const
    {
        return description;
    }

    int getDuration() const { return duration; }
};

class Burn : public Effect
{
private:
    int damagepturn;

public:
    Burn(int dur = 3, int dam = 5) : Effect("Burn", dur, "Deals damage over time as the target is scorched by flames"),
                                     damagepturn(dam) {}

    void apply(Player &player) override;

    int getDamagePerTurn() const
    {
        return damagepturn;
    }
};

class Freeze : public Effect
{
public:
    Freeze(int dur = 2) : Effect("Freeze", dur, "Skip next turn") {}

    void apply(Player &player) override;
};

class Buff : public Effect
{
private:
    string buffType;
    int value;

public:
    Buff(const string &type, int val, int duration = 3) : Effect("Buff (" + type + ")", duration, "Increases " + type + " by " + to_string(val)),
                                                          buffType(type),
                                                          value(val) {}

    void apply(Player &player) override;

    const string &getBuffType() const
    {
        return buffType;
    }

    int getValue() const
    {
        return value;
    }
};

// SKILLS!!

class Skill
{
protected:
    string name;
    int cooldown;
    int currentCoolDown;
    string description;

public:
    Skill(const string &n, int cd, const string &desc)
        : name(n), cooldown(cd), description(desc)
    {
    }
    virtual ~Skill() = default;

    virtual void use(Player &player, Enemy &enemy) = 0; /// MUST APPLY ONCE PLAYER AND ENEMY CLASS IS MAED!!

    bool isAvailaible() const { return currentCoolDown <= 0; }

    void decrementCoolDown()
    {
        if (currentCoolDown > 0)
        {
            currentCoolDown--;
        }
    }

    void resetCooldown()
    {
        currentCoolDown = cooldown;
    }
    const string &getname() const { return name; }

    const string &getdescription() const { return description; }

    int getcurrentCoolDown() const { return currentCoolDown; }

    int getcooldown() const { return cooldown; }
};

class FireBlast : public Skill
{
    int damage;

public:
    FireBlast(int skillDamage = 25) : Skill("FIRE BLAST", 3, "Unleash the fury of the flames"), damage(skillDamage) {}

    void use(Player &player, Enemy &enemy) override;

    int getDamage() { return damage; }
};

class Backstab : public Skill
{
private:
    int damage;

public:
    Backstab(int skillDamage = 40) : Skill("Backstab", 2, "Extra damage if enemy health is below 50%"),
                                     damage(skillDamage) {}

    void use(Player &player, Enemy &enemy) override;

    int getDamage() const
    {
        return damage;
    }
};

class HealingTouch : public Skill
{
private:
    int healAmount;

public:
    HealingTouch(int amount = 30) : Skill("Healing Touch", 4, "Restore health to player"),
                                    healAmount(amount) {}

    void use(Player &player, Enemy &enemy) override;

    int getHealAmount() const
    {
        return healAmount;
    }
};

class ShieldBlock : public Skill
{
public:
    ShieldBlock() : Skill("Shield Block", 3, "Reduces incoming damage for 2 turns") {}

    void use(Player &player, Enemy &enemy) override;
};

// ITEMS!!

class Item
{
protected:
    string name;
    string description;
    int value;

public:
    Item(const string &n, const string &desc, int itemValue = 10) : name(n), description(desc), value(itemValue)
    {
    }

    virtual ~Item() = default;

    virtual void applyEffect(Player &player) = 0; // MUST MAKE ONCE PLAYER CLASS MADE!

    const string &getName() const { return name; }

    const string &getDescription() const { return description; }

    int getValue() const { return value; }
};

class Weapon : public Item
{
    int bonusDamage;

public:
    Weapon(const string &n, int damage, const string &desc = "Increases Attack Damage") : Item(n, desc, damage * 10), bonusDamage(damage) {}

    void applyEffect(Player &player) override;

    int getBonusDamage() const { return bonusDamage; }
};

class Armor : public Item
{
    int defenseBoost;

public:
    Armor(const string &n, int def, const string &description = "Protective gear that reduces incoming damage, increasing a player's survivability in battle") : Item(name, description, def * 10), defenseBoost(def) {}

    void applyEffect(Player &player) override;

    int getDefenseBoost() const { return defenseBoost; }
};

class HealthPotion : public Item
{
private:
    int healAmount;

public:
    HealthPotion(int amount = 25) : Item("Health Potion", "A revitalizing brew that restores a portion of your health", amount / 2),
                                    healAmount(amount) {}

    void applyEffect(Player &player) override;

    int getHealAmount() const
    {
        return healAmount;
    }
};

class FireScroll : public Item
{
public:
    FireScroll() : Item("Fire Scroll", "Unleashes searing flames to scorch enemies with a burning curse", 30) {}

    void applyEffect(Player &player) override;
};

class ThunderAmulet : public Item
{
private:
    int damageBoost;

public:
    ThunderAmulet(int boost = 15) : Item("Thunder Amulet", "Unleashes a lightning-infused attack, amplifying your damage with each strike", boost * 3),
                                    damageBoost(boost) {}

    void applyEffect(Player &player) override;

    int getDamageBoost() const
    {
        return damageBoost;
    }
};

class MindCrystal : public Item
{
public:
    MindCrystal() : Item("Mind Crystal", "Empowers the user with mental fortitude, preventing any status effects for one turn", 50) {}

    void applyEffect(Player &player) override;
};

class Artifact : public Item
{
private:
    string abilityName;
    function<void(Player &)> effect;

public:
    Artifact(const string &name, const string &ability,
             const string &desc, const function<void(Player &)> &effectFn) : Item(name, desc, 100),
                                                                             abilityName(ability),
                                                                             effect(effectFn) {}

    void applyEffect(Player &player) override;

    const string &getAbilityName() const
    {
        return abilityName;
    }
};

// INVENTORY

class Inventory
{
private:
    vector<Item *> items;
    int capacity;

public:
    Inventory(int maxcap = 20) : capacity(20) {}

    ~Inventory()
    {
        for (auto i : items)
        {
            delete i;
        }
        items.clear();
    }

    bool addItem(Item *item)
    {
        if (items.size() < capacity && item)
        {
            items.push_back(item);
            return true;
        }
        return false;
    }

    Item *useItem(const string &name)
    {
        for (auto &item : items)
        {
            if (item && item->getName() == name)
            {
                return item;
            }
        }
        return nullptr;
    }

    bool removeItem(const string &name)
    {
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            if (*it && (*it)->getName() == name)
            {
                delete *it;
                items.erase(it);
                return true;
            }
        }
        return false;
    }

    void listItems() const
    {
        if (items.empty())
        {
            cout << "Inventory is Empty, Collect Items and LEVEL UP!" << endl
                 << endl;
        }

        cout << "----INVENTORY [" << items.size() << " / " << capacity << "]----" << endl
             << endl;

        int index = 1;
        for (auto item : items)
        {
            if (item)
            {
                cout << index++ << item->getName() << endl;
                cout << item->getDescription() << endl;
                cout << "Value [" << item->getValue() << "]" << endl;
            }
        }
        cout << "-------------------------------------------" << endl
             << endl;
    }

    const vector<Item *> &getItems() const
    {
        return items;
    }

    size_t getItemCount()
    {
        return items.size();
    }

    bool isFull() const
    {
        return items.size() >= capacity;
    }
};

class Player
{
    string name;
    int health;
    int maxHealth;
    int damage;
    int level;
    int xp;
    int xpToNextLevel;
    Inventory inventory;
    vector<Effect *> statusEffects;
    vector<Skill *> skills;
    HeartSystem hearts;
    Stack levelStack;
    int defense;
    Weapon *equippedWeapon;
    Armor *equippedArmor;
    bool immunetoStatusEffects;
    int damageReduction;

public:
    Player(const string &playerName = "GalacticShade") : name(playerName), health(100), maxHealth(100), damage(10), level(1), xp(0), xpToNextLevel(100),
                                                         inventory(), statusEffects(), skills(), hearts(5), levelStack(), defense(0), equippedWeapon(nullptr),
                                                         equippedArmor(nullptr), immunetoStatusEffects(false), damageReduction(0)
    {
        // BASIC SKILLS PLAYER WILL HAVE
        skills.push_back(new FireBlast());
        skills.push_back(new Backstab());
        skills.push_back(new HealingTouch());
        skills.push_back(new ShieldBlock());

        // STARTING INFORMATION
        if (!levelStack.isEmpty())
        {
            level = levelStack.getCurrLevel();
            xpToNextLevel = levelStack.getreqXPForNextLevel();
        }
    }

    ~Player()
    {
        for (auto effect : statusEffects)
        {
            delete effect;
        }
        statusEffects.clear();
        for (auto skill : skills)
        {
            delete skill;
        }
        skills.clear();
    }

    int attack(Enemy *enemy)
    {
        if (!enemy)
        {
            return 0;
        }
        int totalDamage = damage;
        if (equippedWeapon)
        {
            totalDamage += equippedWeapon->getBonusDamage();
        }

        bool isCritical = gen.critical();
        if (isCritical)
        {
            totalDamage *= 12;
            gameLogger.logEvent(name + " STRIKES TRUEE! A devastating critical hit shakes the enemy to its core!");
        }

        enemy->takeDamage(totalDamage); // INCLUDE THIS METHOD IN ENENMY CLASS

        gameLogger.logEvent(name + "'s strike carved through, inflicting " + to_string(totalDamage) + " damage!");
        return totalDamage;
    }

    void takeDamage(int amt)
    {
        // DEFENSE
        if (equippedArmor)
        {
            amt = max(1, amt - equippedArmor->getDefenseBoost());
        }
        // DAMAGE REDUCTION FROM SKILLS/EFFECT
        if (damageReduction > 0)
        {
            int reducedAmount = static_cast<int>(amt * (1 - (damageReduction / 100)));
            amt = max(1, reducedAmount);
        }

        health -= amt;

        if (amt > 0)
        {
            if (amt >= 20)
            {
                hearts.loseHeart();
            }
            gameLogger.logEvent(name + " takes a brutal hit for " + to_string(amt) + " damage!");
        }

        if (health <= 0)
        {
            health = 0;
            gameLogger.logEvent("The hero falls! " + name + " has been defeated!");
        }
    }

    void heal(int amt)
    {
        int oldhealth = health;
        health = min(health + amt, maxHealth);
        int actualHeal = health - oldhealth; // HOW MUCGH HEALTH WAS ADDED

        if (actualHeal >= 30)
        {
            hearts.gainHeart();
        }

        gameLogger.logEvent(name + " is enveloped by a soft glow, healing for " + to_string(actualHeal) + " health!");
    }

    void gainXP(int amt)
    {
        xp += amt;
        gameLogger.logEvent(name + " triumphs in battle and gains " + to_string(amt) + " XP!");

        while (xp >= xpToNextLevel)
        {
            levelUp();
        }
    }

    void levelUp()
    {
        if (levelStack.isEmpty())
        {
            gameLogger.logEvent("The path is now clear — " + name + " has reached the maximum level, their legend solidified!");
            return;
        }

        // remove currwent level from stack

        LevelNode *currLevel = levelStack.levelUp();
        if (currLevel)
        {
            level = currLevel->level;
            xp -= xpToNextLevel;

            if (!levelStack.isEmpty())
            {
                xpToNextLevel = levelStack.getreqXPForNextLevel();
            }
            else
            {
                xpToNextLevel = -1; // indicate no more levels
            }

            maxHealth += 20;
            health = maxHealth;
            damage += 5;

            // RESTORE HEAARTS

            while (hearts.getCurrHearts() < hearts.getMaxHearts())
            {
                hearts.gainHeart();
            }

            // RESET ALL SKILLS COOLDOWN

            for (auto skill : skills)
            {
                skill->decrementCoolDown();
            }

            gameLogger.logEvent("The journey continues! " + name + " has reached Level " + to_string(level) + "!");
            delete currLevel;
        }
    }

    void addStatusEffect(Effect *effect)
    {
        if (!effect)
        {
            return;
        }

        if (immunetoStatusEffects)
        {
            gameLogger.logEvent("The power of " + effect->getName() + " has no hold over " + name + "!");
            delete effect;
            return;
        }

        bool found = false;

        for (auto &existing : statusEffects)
        {
            if (existing->getName() == effect->getName())
            {
                delete existing;
                existing = effect;
                found = true;
                break;
            }
        }

        if (!found)
        {
            statusEffects.push_back(effect);
        }

        gameLogger.logEvent(name + " gains the power of " + effect->getName() + " for the next " + to_string(effect->getDuration()) + " turns.");
    }

    void useSkill(Skill *skill, Enemy *enemy)
    {
        if (!skill || !enemy || !skill->isAvailaible())
        {
            return;
        }

        skill->use(*this, *enemy);

        skill->resetCooldown();

        gameLogger.logEvent(name + " unleashed the power of " + skill->getname() + "!");
    }

    bool addItem(Item *item)
    {
        if (inventory.addItem(item))
        {
            gameLogger.logEvent("name acquired the mystical " + item->getName() + ".");
            return true;
        }
        return false;
    }

    void useItem(const string &itemName)
    {
        Item *item = inventory.useItem(itemName);

        if (item)
        {

            item->applyEffect(*this);
            gameLogger.logEvent("name activated " + item->getName() + " in a moment of need.");
        }
        else
        {
            gameLogger.logEvent("You fail to find " + itemName + " within your pack.");
        }
    }

    void equipWeapon(Weapon *weapon)
    {
        if (!weapon)
        {
            return;
        }

        // UNEQUIP CURRENT WEAPON
        if (equippedWeapon)
        {
            gameLogger.logEvent("name detached the mystical " + equippedWeapon->getName() + " from their arm.");
        }

        equippedWeapon = weapon;
        gameLogger.logEvent("name gripped the ancient " + weapon->getName() + ", a weapon of untold power.");
    }

    void equipArmor(Armor *armor)
    {
        if (!armor)
            return;

        // UNEQUIPN CURRENT ARMOR
        if (equippedArmor)
        {
            gameLogger.logEvent(name + "  unbound the mystical " + equippedArmor->getName() + ", letting its magic rest.");
        }

        equippedArmor = armor;
        gameLogger.logEvent(name + "invoked the ancient rite and the " + armor->getName() + " shimmered into place.");
    }

    void setImmuneToStatusEffects(bool immune)
    {
        immunetoStatusEffects = immune;
        if (immune)
        {
            gameLogger.logEvent(name + " is now shielded from all vile afflictions!");
        }
    }

    void setDamageReduction(int reduction)
    {
        damageReduction = reduction;
        if (reduction > 0)
        {
            gameLogger.logEvent(name + " is cloaked in protective magic , damage reduced by " + to_string(reduction) + "%!");
        }
    }

    void applyStatusEffects()
    {
        // APPLY ALL EFECTS
        for (auto effect : statusEffects)
        {
            effect->apply(*this);
        }

        auto it = statusEffects.begin();
        while (it != statusEffects.end())
        {
            if ((*it)->isExpired())
            {
                gameLogger.logEvent(name + "'s " + (*it)->getName() + " has faded into the ether.");
                delete *it;
                it = statusEffects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void decrementCooldowns()
    {
        for (auto skill : skills)
        {
            skill->decrementCoolDown();
        }
    }

    const string &getName() const { return name; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getDamage() const { return damage; }
    int getLevel() const { return level; }
    int getXP() const { return xp; }
    int getXPToNextLevel() const { return xpToNextLevel; }
    const Inventory &getInventory() const { return inventory; }
    Inventory &getInventoryRef() { return inventory; }
    const vector<Effect *> &getStatusEffects() const { return statusEffects; }
    const vector<Skill *> &getSkills() const { return skills; }
    const HeartSystem &getHearts() const { return hearts; }
    HeartSystem &getHeartsRef() { return hearts; } // CAN MODIFY wherever necessary
    int getDefense() const { return defense; }
    // SETTERS
    void setDamage(int newDamage) { damage = newDamage; }
    void setDefense(int newDefense) { defense = newDefense; }

    void displayStats() const
    {
        cout << "NAME: " << name << "[LEVEL : " << level << " ] " << endl
             << endl;
        cout << "HEALTH: " << health << " / " << maxHealth << endl;
        hearts.display();
        cout << "DAMAGE: " << damage;
        if (equippedWeapon)
        {
            cout << " + " << equippedWeapon->getBonusDamage() << "(WEAPON)";
        }
        cout << endl
             << endl;

        if (equippedArmor)
        {
            cout << " + " << equippedArmor->getDefenseBoost() << "(ARMOR)";
        }
        cout << endl
             << endl;

        cout << "XP : " << xp << " / " << xpToNextLevel << endl;

        if (!statusEffects.empty())
        {
            cout << "STATUS EFFECTS: " << endl;
            for (const auto &effect : statusEffects)
            {
                cout << " > " << effect->getName() << "[" << effect->getDuration() << "TURNS ]" << endl;
            }
        }

        cout << "SKILLS: " << endl
             << endl;
        for (const auto &skill : skills)
        {
            cout << " > " << skill->getname() << " : " << skill->getdescription();
            if (skill->isAvailaible())
            {
                cout << "[READY]" << endl
                     << endl;
            }
            else
            {
                cout << "[COOLDOWN : " << skill->getcurrentCoolDown() << " ] " << endl
                     << endl;
            }
        }

        if (equippedWeapon)
        {
            cout << "Weapon: " << equippedWeapon->getName()
                 << " [ + " << equippedWeapon->getBonusDamage() << " damage ]" << endl;
        }

        if (equippedArmor)
        {
            cout << "Armor: " << equippedArmor->getName()
                 << " [+" << equippedArmor->getDefenseBoost() << " defense]" << endl;
        }

        cout << "--------------------------------------------------" << endl;
    }
};

class Enemy
{
protected:
    string name;
    int health;
    int maxHealth;
    int damage;
    int xpReward;
    vector<Effect *> statusEffects;
    bool isBoss;

public:
    Enemy(const string &eneName, int health, int damage, int reward, bool boss = false)
        : name(eneName), health(health), maxHealth(health), damage(damage), xpReward(reward), statusEffects(), isBoss(boss)
    {
    }

    virtual ~Enemy()
    {
        for (auto effect : statusEffects)
        {
            delete effect;
        }
        statusEffects.clear();
    }

    virtual int attack(Player &player)
    {
        int attackDamage = damage;

        // VARIATION
        int variation = gen.getRandom(-20, 20);
        attackDamage = max(1, attackDamage + (attackDamage * variation / 100));

        player.takeDamage(attackDamage);

        gameLogger.logEvent(name + " unleashes a fierce blow, dealing " + to_string(attackDamage) + " damage!");
        return attackDamage;
    }

    virtual void takeDamage(int amount)
    {
        health -= amount;

        if (health <= 0)
        {
            health = 0;
            gameLogger.logEvent("Victory! " + name + " has been crushed in combat.");
        }
    }

    virtual void useAbility(Player &player)
    {
        // BASIC ENTITY ATTACKS;
        attack(player);
    }

    void addStatusEffect(Effect *effect)
    {
        if (!effect)
            return;

        bool replaced = false;

        for (auto &existing : statusEffects)
        {
            if (existing->getName() == effect->getName())
            {
                delete existing;
                existing = effect;
                replaced = true;
                break;
            }
        }

        if (!replaced)
        {
            statusEffects.push_back(effect);
        }

        gameLogger.logEvent(name + " gained " + effect->getName() + " effect for " +
                            to_string(effect->getDuration()) + " turns");
    }

    void applyStatusEffects()
    {
        // Apply burn status effects to take damage over time
        for (auto effect : statusEffects)
        {
            if (effect->getName() == "Burn")
            {
                int burnDamage = 5;
                health -= burnDamage;
                gameLogger.logEvent(name + " writhes in pain, burning for " + to_string(burnDamage) + " damage!");
            }
        }

        // DECREMENT DURATION AND REMOVE EXPIRED EFFECTS
        auto it = statusEffects.begin();
        while (it != statusEffects.end())
        {
            (*it)->Decrementduration();
            if ((*it)->isExpired())
            {
                gameLogger.logEvent(name + "'s " + (*it)->getName() + " effect has expired");
                delete *it;
                it = statusEffects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    const string &getName() const { return name; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getDamage() const { return damage; }
    int getXPReward() const { return xpReward; }
    const vector<Effect*> &getStatusEffects() const { return statusEffects; }
    bool getIsBoss() const { return isBoss; }

    virtual string getType() const
    {
        return "Enemy";
    }

    void displayStats() const {
    cout << "----- " << name << " [" << getType() << "] -----" << endl;
    cout << "Health: " << health << " / " << maxHealth << endl;
    cout << "Damage: " << damage << endl;
    
    if (!statusEffects.empty()) {
        cout << "Status Effects:" << endl;
        for (const auto& effect : statusEffects) {
            cout << " > " << effect->getName() 
                 << " [" << effect->getDuration() << " turns ]" << endl;
        }
    }
    
    cout << "XP Reward: " << xpReward << endl;
    cout << "---------------------------" << endl;
}
};