#pragma once

#include <string>
#include <unordered_map>

//class Ant;

class Blackboard
{

public:

    // Sets; Will set key to value with no safe checking;
    void SetKeyValue(std::string key_, int value_);
    void SetKeyValue(std::string key_, float value_);
    void SetKeyValue(std::string key_, double value_);
    void SetKeyValue(std::string key_, bool value_);
    void SetKeyValue(std::string key_, std::string value_);
    // void SetKeyValue(std::string key_, Ant* value_);

    // Gets; Will retrieve the value from the key, if non-existent it will insert the basic value and return that;
    int GetIntFromKey(std::string key_);
    float GetFloatFromKey(std::string key_);
    double GetDoubleFromKey(std::string key_);
    bool GetBoolFromKey(std::string key_);
    std::string GetStringFromKey(std::string key_);
    // Ant* GetAntFromKey(std::string key_);

    // Has; Returns true or false if the key is found in the map;
    bool HasIntFromKey(const std::string& key_) const;
    bool HasFloatFromKey(const std::string& key_) const;
    bool HasDoubleFromKey(const std::string& key_) const;
    bool HasBoolFromKey(const std::string& key_) const;
    bool HasStringFromKey(const std::string& key_) const;
    //bool HasAntUnitFromKey(const std::string& key_) const;

protected:

    std::unordered_map<std::string, int> m_ints;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, double> m_doubles;
    std::unordered_map<std::string, bool> m_bools;
    std::unordered_map<std::string, std::string> m_strings;
    //std::unordered_map<std::string, Ant*> m_ants;

};