#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "utils.h"

class JSONHandler {
private:
    rapidjson::Document document;

public:
    JSONHandler();
    explicit JSONHandler(const std::string& jsonString);

    bool parse(const std::string& jsonString);
    std::string serialize() const;

    template <typename T>
    void setValue(const std::string& key, const T& value);

    template <typename T>
    T getValue(const std::string& key, const T& defaultValue = T()) const;

    template <typename T>
    void setVector(const std::string& key, const std::vector<T>& values);

    template <typename T>
    std::vector<T> getVector(const std::string& key, const std::vector<T>& defaultValue = {}) const;

    bool hasKey(const std::string& key) const;
};


inline JSONHandler::JSONHandler() {
    document.SetObject();
}

inline JSONHandler::JSONHandler(const std::string& jsonString) {
    parse(jsonString);
}

inline bool JSONHandler::parse(const std::string& jsonString) {
    document.Parse(jsonString.c_str());
    if (document.HasParseError()) {
        Utils::LOG_ERROR("JSON Parse Error");
        document.SetObject();
        return false;
    }
    return true;
}

inline std::string JSONHandler::serialize() const {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

template <typename T>
inline void JSONHandler::setValue(const std::string &key, const T &value) {
    if (!document.IsObject()) {
        document.SetObject();
    }

    rapidjson::Value k(key.c_str(), document.GetAllocator());

    if constexpr (std::is_same_v<T, std::string>) {
        rapidjson::Value v;
        v.SetString(value.c_str(), static_cast<rapidjson::SizeType>(value.length()), document.GetAllocator());
        document.AddMember(k, v, document.GetAllocator());
    } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        document.AddMember(k, value, document.GetAllocator());
    } else {
        static_assert(!sizeof(T), "Unsupported data type for JSONHandler::setValue");
    }
}

template <typename T>
inline T JSONHandler::getValue(const std::string &key, const T &defaultValue) const {
    if (!document.HasMember(key.c_str())) {
        return defaultValue;
    }

    const rapidjson::Value &value = document[key.c_str()];

    if constexpr (std::is_same_v<T, std::string>) {
        if (value.IsString()) return std::string(value.GetString());
    } else if constexpr (std::is_integral_v<T>) {
        if (value.IsInt()) return value.GetInt();
    } else if constexpr (std::is_floating_point_v<T>) {
        if (value.IsDouble()) return value.GetDouble();
    }

    return defaultValue;
}

template <typename T>
inline void JSONHandler::setVector(const std::string &key, const std::vector<T> &values) {
    if (!document.IsObject()) {
        document.SetObject();
    }

    rapidjson::Value k(key.c_str(), document.GetAllocator());
    rapidjson::Value array(rapidjson::kArrayType);

    for (const auto &val : values) {
        if constexpr (std::is_same_v<T, std::string>) {
            rapidjson::Value v;
            v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.length()), document.GetAllocator());
            array.PushBack(v, document.GetAllocator());
        } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            array.PushBack(val, document.GetAllocator());
        } else {
            static_assert(!sizeof(T), "Unsupported vector type for JSONHandler::setVector");
        }
    }

    document.AddMember(k, array, document.GetAllocator());
}

template <typename T>
inline std::vector<T> JSONHandler::getVector(const std::string &key, const std::vector<T> &defaultValue) const {
    if (!document.HasMember(key.c_str()) || !document[key.c_str()].IsArray()) {
        return defaultValue;
    }

    const rapidjson::Value &array = document[key.c_str()];
    std::vector<T> result;

    for (auto &v : array.GetArray()) {
        if constexpr (std::is_same_v<T, std::string>) {
            if (v.IsString()) result.push_back(std::string(v.GetString()));
        } else if constexpr (std::is_integral_v<T>) {
            if (v.IsInt()) result.push_back(v.GetInt());
        } else if constexpr (std::is_floating_point_v<T>) {
            if (v.IsDouble()) result.push_back(v.GetDouble());
        }
    }

    return result;
}

inline bool JSONHandler::hasKey(const std::string &key) const {
    return document.HasMember(key.c_str());
}

#endif  // JSON_HANDLER_H
