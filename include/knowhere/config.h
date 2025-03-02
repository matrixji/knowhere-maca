// Copyright (C) 2019-2023 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License.

#ifndef CONFIG_H
#define CONFIG_H

#include <omp.h>

#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "knowhere/expected.h"
#include "knowhere/log.h"
#include "nlohmann/json.hpp"

namespace knowhere {

typedef nlohmann::json Json;

#ifndef CFG_INT
#define CFG_INT std::optional<int32_t>
#endif

#ifndef CFG_STRING
#define CFG_STRING std::optional<std::string>
#endif

#ifndef CFG_FLOAT
#define CFG_FLOAT std::optional<float>
#endif

#ifndef CFG_LIST
#define CFG_LIST std::optional<std::list<int>>
#endif

#ifndef CFG_BOOL
#define CFG_BOOL std::optional<bool>
#endif

template <typename T>
struct Entry {};

enum PARAM_TYPE {
    TRAIN = 1 << 0,
    SEARCH = 1 << 1,
    RANGE_SEARCH = 1 << 2,
    FEDER = 1 << 3,
    DESERIALIZE = 1 << 4,
    DESERIALIZE_FROM_FILE = 1 << 5,
};

template <>
struct Entry<CFG_STRING> {
    explicit Entry<CFG_STRING>(CFG_STRING* v) {
        val = v;
        type = 0x0;
        default_val = std::nullopt;
        desc = std::nullopt;
    }
    Entry<CFG_STRING>() {
        val = nullptr;
        type = 0x0;
        default_val = std::nullopt;
        desc = std::nullopt;
    }
    CFG_STRING* val;
    uint32_t type;
    std::optional<CFG_STRING::value_type> default_val;
    std::optional<std::string> desc;
    bool allow_empty_without_default = false;
};

template <>
struct Entry<CFG_FLOAT> {
    explicit Entry<CFG_FLOAT>(CFG_FLOAT* v) {
        val = v;
        default_val = std::nullopt;
        type = 0x0;
        range = std::nullopt;
        desc = std::nullopt;
    }
    Entry<CFG_FLOAT>() {
        val = nullptr;
        default_val = std::nullopt;
        type = 0x0;
        range = std::nullopt;
        desc = std::nullopt;
    }

    CFG_FLOAT* val;
    std::optional<CFG_FLOAT::value_type> default_val;
    uint32_t type;
    std::optional<std::pair<CFG_FLOAT::value_type, CFG_FLOAT::value_type>> range;
    std::optional<std::string> desc;
    bool allow_empty_without_default = false;
};

template <>
struct Entry<CFG_INT> {
    explicit Entry<CFG_INT>(CFG_INT* v) {
        val = v;
        default_val = std::nullopt;
        type = 0x0;
        range = std::nullopt;
        desc = std::nullopt;
    }
    Entry<CFG_INT>() {
        val = nullptr;
        default_val = std::nullopt;
        type = 0x0;
        range = std::nullopt;
        desc = std::nullopt;
    }

    CFG_INT* val;
    std::optional<CFG_INT::value_type> default_val;
    uint32_t type;
    std::optional<std::pair<CFG_INT::value_type, CFG_INT::value_type>> range;
    std::optional<std::string> desc;
    bool allow_empty_without_default = false;
};

template <>
struct Entry<CFG_LIST> {
    explicit Entry<CFG_LIST>(CFG_LIST* v) {
        val = v;
        default_val = std::nullopt;
        type = 0x0;
        desc = std::nullopt;
    }

    Entry<CFG_LIST>() {
        val = nullptr;
        default_val = std::nullopt;
        type = 0x0;
        desc = std::nullopt;
    }

    CFG_LIST* val;
    std::optional<CFG_LIST::value_type> default_val;
    uint32_t type;
    std::optional<std::string> desc;
    bool allow_empty_without_default = false;
};

template <>
struct Entry<CFG_BOOL> {
    explicit Entry<CFG_BOOL>(CFG_BOOL* v) {
        val = v;
        default_val = std::nullopt;
        type = 0x0;
        desc = std::nullopt;
    }

    Entry<CFG_BOOL>() {
        val = nullptr;
        default_val = std::nullopt;
        type = 0x0;
        desc = std::nullopt;
    }

    CFG_BOOL* val;
    std::optional<CFG_BOOL::value_type> default_val;
    uint32_t type;
    std::optional<std::string> desc;
    bool allow_empty_without_default = false;
};

template <typename T>
class EntryAccess {
 public:
    EntryAccess(Entry<T>* entry) : entry(entry){};

    EntryAccess&
    set_default(const typename T::value_type dft) {
        entry->default_val = dft;
        *entry->val = dft;
        return *this;
    }

    EntryAccess&
    set_range(typename T::value_type a, typename T::value_type b) {
        entry->range = std::make_pair(a, b);
        return *this;
    }

    EntryAccess&
    allow_empty_without_default() {
        entry->allow_empty_without_default = true;
        return *this;
    }

    EntryAccess&
    description(const std::string& desc) {
        entry->desc = desc;
        return *this;
    }

    EntryAccess&
    for_train() {
        entry->type |= PARAM_TYPE::TRAIN;
        return *this;
    }

    EntryAccess&
    for_search() {
        entry->type |= PARAM_TYPE::SEARCH;
        return *this;
    }

    EntryAccess&
    for_range_search() {
        entry->type |= PARAM_TYPE::RANGE_SEARCH;
        return *this;
    }

    EntryAccess&
    for_feder() {
        entry->type |= PARAM_TYPE::FEDER;
        return *this;
    }

    EntryAccess&
    for_deserialize() {
        entry->type |= PARAM_TYPE::DESERIALIZE;
        return *this;
    }

    EntryAccess&
    for_deserialize_from_file() {
        entry->type |= PARAM_TYPE::DESERIALIZE_FROM_FILE;
        return *this;
    }

    EntryAccess&
    for_train_and_search() {
        entry->type |= PARAM_TYPE::TRAIN;
        entry->type |= PARAM_TYPE::SEARCH;
        entry->type |= PARAM_TYPE::RANGE_SEARCH;
        return *this;
    }

 private:
    Entry<T>* entry;
};

class Config {
 public:
    static Status
    FormatAndCheck(const Config& cfg, Json& json, std::string* const err_msg = nullptr);

    static Status
    Load(Config& cfg, const Json& json, PARAM_TYPE type, std::string* const err_msg = nullptr) {
        for (const auto& it : cfg.__DICT__) {
            const auto& var = it.second;

            if (const Entry<CFG_INT>* ptr = std::get_if<Entry<CFG_INT>>(&var)) {
                if (!(type & ptr->type)) {
                    continue;
                }
                if (json.find(it.first) == json.end() && !ptr->default_val.has_value()) {
                    if (ptr->allow_empty_without_default) {
                        continue;
                    }
                    LOG_KNOWHERE_ERROR_ << "Invalid param [" << it.first << "] in json.";
                    if (err_msg) {
                        *err_msg = std::string("invalid param ") + it.first;
                    }
                    return Status::invalid_param_in_json;
                }
                if (json.find(it.first) == json.end()) {
                    *ptr->val = ptr->default_val;
                    continue;
                }
                if (!json[it.first].is_number_integer()) {
                    LOG_KNOWHERE_ERROR_ << "Type conflict in json: param [" << it.first << "] should be integer.";
                    if (err_msg) {
                        *err_msg = std::string("param ") + it.first + " should be integer";
                    }
                    return Status::type_conflict_in_json;
                }
                if (ptr->range.has_value()) {
                    if (json[it.first].get<long>() > std::numeric_limits<CFG_INT::value_type>::max()) {
                        LOG_KNOWHERE_ERROR_ << "Arithmetic overflow: param [" << it.first << "] should be at most "
                                            << std::numeric_limits<CFG_INT::value_type>::max();
                        if (err_msg) {
                            *err_msg = std::string("param ") + it.first + " should be at most 2147483647";
                        }
                        return Status::arithmetic_overflow;
                    }
                    CFG_INT::value_type v = json[it.first];
                    if (ptr->range.value().first <= v && v <= ptr->range.value().second) {
                        *ptr->val = v;
                    } else {
                        LOG_KNOWHERE_ERROR_ << "Out of range in json: param [" << it.first << "] should be in ["
                                            << ptr->range.value().first << ", " << ptr->range.value().second << "].";
                        if (err_msg) {
                            *err_msg = std::string("param ") + it.first + " out of range " + "[ " +
                                       std::to_string(ptr->range.value().first) + "," +
                                       std::to_string(ptr->range.value().second) + " ]";
                        }
                        return Status::out_of_range_in_json;
                    }
                } else {
                    *ptr->val = json[it.first];
                }
            }

            if (const Entry<CFG_FLOAT>* ptr = std::get_if<Entry<CFG_FLOAT>>(&var)) {
                if (!(type & ptr->type)) {
                    continue;
                }
                if (json.find(it.first) == json.end() && !ptr->default_val.has_value()) {
                    if (ptr->allow_empty_without_default) {
                        continue;
                    }
                    LOG_KNOWHERE_ERROR_ << "Invalid param [" << it.first << "] in json.";
                    if (err_msg) {
                        *err_msg = std::string("invalid param ") + it.first;
                    }

                    return Status::invalid_param_in_json;
                }
                if (json.find(it.first) == json.end()) {
                    *ptr->val = ptr->default_val;
                    continue;
                }
                if (!json[it.first].is_number()) {
                    LOG_KNOWHERE_ERROR_ << "Type conflict in json: param [" << it.first << "] should be a number.";
                    if (err_msg) {
                        *err_msg = std::string("param ") + it.first + " should be a number";
                    }

                    return Status::type_conflict_in_json;
                }
                if (ptr->range.has_value()) {
                    if (json[it.first].get<double>() > std::numeric_limits<CFG_FLOAT::value_type>::max()) {
                        LOG_KNOWHERE_ERROR_ << "Arithmetic overflow: param [" << it.first << "] should be at most "
                                            << std::numeric_limits<CFG_FLOAT::value_type>::max();
                        if (err_msg) {
                            *err_msg = std::string("param ") + it.first + " should be at most 3.402823e+38";
                        }

                        return Status::arithmetic_overflow;
                    }
                    CFG_FLOAT::value_type v = json[it.first];
                    if (ptr->range.value().first <= v && v <= ptr->range.value().second) {
                        *ptr->val = v;
                    } else {
                        LOG_KNOWHERE_ERROR_ << "Out of range in json: param [" << it.first << "] should be in ["
                                            << ptr->range.value().first << ", " << ptr->range.value().second << "].";
                        if (err_msg) {
                            *err_msg = std::string("param ") + it.first + " out of range " + "[ " +
                                       std::to_string(ptr->range.value().first) + "," +
                                       std::to_string(ptr->range.value().second) + " ]";
                        }

                        return Status::out_of_range_in_json;
                    }
                } else {
                    *ptr->val = json[it.first];
                }
            }

            if (const Entry<CFG_STRING>* ptr = std::get_if<Entry<CFG_STRING>>(&var)) {
                if (!(type & ptr->type)) {
                    continue;
                }
                if (json.find(it.first) == json.end() && !ptr->default_val.has_value()) {
                    if (ptr->allow_empty_without_default) {
                        continue;
                    }
                    LOG_KNOWHERE_ERROR_ << "Invalid param [" << it.first << "] in json.";
                    if (err_msg) {
                        *err_msg = std::string("invalid param ") + it.first;
                    }
                    return Status::invalid_param_in_json;
                }
                if (json.find(it.first) == json.end()) {
                    *ptr->val = ptr->default_val;
                    continue;
                }
                if (!json[it.first].is_string()) {
                    LOG_KNOWHERE_ERROR_ << "Type conflict in json: param [" << it.first << "] should be a string.";
                    if (err_msg) {
                        *err_msg = std::string("param ") + it.first + " should be a string";
                    }
                    return Status::type_conflict_in_json;
                }
                *ptr->val = json[it.first];
            }

            if (const Entry<CFG_LIST>* ptr = std::get_if<Entry<CFG_LIST>>(&var)) {
                if (!(type & ptr->type)) {
                    continue;
                }
                if (json.find(it.first) == json.end() && !ptr->default_val.has_value()) {
                    if (ptr->allow_empty_without_default) {
                        continue;
                    }
                    LOG_KNOWHERE_ERROR_ << "Invalid param [" << it.first << "] in json.";
                    if (err_msg) {
                        *err_msg = std::string("invalid param ") + it.first;
                    }

                    return Status::invalid_param_in_json;
                }
                if (json.find(it.first) == json.end()) {
                    *ptr->val = ptr->default_val;
                    continue;
                }
                if (!json[it.first].is_array()) {
                    LOG_KNOWHERE_ERROR_ << "Type conflict in json: param [" << it.first << "] should be an array.";
                    if (err_msg) {
                        *err_msg = std::string("param ") + it.first + " should be an array";
                    }

                    return Status::type_conflict_in_json;
                }
                *ptr->val = CFG_LIST::value_type();
                for (auto&& i : json[it.first]) {
                    ptr->val->value().push_back(i);
                }
            }

            if (const Entry<CFG_BOOL>* ptr = std::get_if<Entry<CFG_BOOL>>(&var)) {
                if (!(type & ptr->type)) {
                    continue;
                }
                if (json.find(it.first) == json.end() && !ptr->default_val.has_value()) {
                    if (ptr->allow_empty_without_default) {
                        continue;
                    }
                    LOG_KNOWHERE_ERROR_ << "Invalid param [" << it.first << "] in json.";
                    if (err_msg) {
                        *err_msg = std::string("invalid param ") + it.first;
                    }

                    return Status::invalid_param_in_json;
                }
                if (json.find(it.first) == json.end()) {
                    *ptr->val = ptr->default_val;
                    continue;
                }
                if (!json[it.first].is_boolean()) {
                    LOG_KNOWHERE_ERROR_ << "Type conflict in json: param [" << it.first << "] should be a boolean.";
                    if (err_msg) {
                        *err_msg = std::string("param ") + it.first + " should be a boolean";
                    }

                    return Status::type_conflict_in_json;
                }
                *ptr->val = json[it.first];
            }
        }

        return Status::success;
    }

    virtual ~Config() {
    }

    using VarEntry =
        std::variant<Entry<CFG_STRING>, Entry<CFG_FLOAT>, Entry<CFG_INT>, Entry<CFG_LIST>, Entry<CFG_BOOL>>;
    std::unordered_map<std::string, VarEntry> __DICT__;
};

#define KNOHWERE_DECLARE_CONFIG(CONFIG) CONFIG()

#define KNOWHERE_CONFIG_DECLARE_FIELD(PARAM)                                                             \
    __DICT__[#PARAM] = knowhere::Config::VarEntry(std::in_place_type<Entry<decltype(PARAM)>>, &PARAM);   \
    EntryAccess<decltype(PARAM)> PARAM##_access(std::get_if<Entry<decltype(PARAM)>>(&__DICT__[#PARAM])); \
    PARAM##_access

const float defaultRangeFilter = 1.0f / 0.0;

class BaseConfig : public Config {
 public:
    CFG_STRING metric_type;
    CFG_INT k;
    CFG_INT num_build_thread;
    CFG_FLOAT radius;
    CFG_FLOAT range_filter;
    CFG_BOOL trace_visit;
    CFG_BOOL enable_mmap;
    CFG_BOOL for_tuning;
    KNOHWERE_DECLARE_CONFIG(BaseConfig) {
        KNOWHERE_CONFIG_DECLARE_FIELD(metric_type).set_default("L2").description("metric type").for_train_and_search();
        KNOWHERE_CONFIG_DECLARE_FIELD(k)
            .set_default(10)
            .description("search for top k similar vector.")
            .set_range(1, std::numeric_limits<CFG_INT::value_type>::max())
            .for_search();
        KNOWHERE_CONFIG_DECLARE_FIELD(num_build_thread)
            .description("index thread limit for build.")
            .allow_empty_without_default()
            .set_range(1, std::thread::hardware_concurrency())
            .for_train();
        KNOWHERE_CONFIG_DECLARE_FIELD(radius)
            .set_default(0.0)
            .description("radius for range search")
            .for_range_search();
        KNOWHERE_CONFIG_DECLARE_FIELD(range_filter)
            .set_default(defaultRangeFilter)
            .description("result filter for range search")
            .for_range_search();
        KNOWHERE_CONFIG_DECLARE_FIELD(trace_visit)
            .set_default(false)
            .description("trace visit for feder")
            .for_search()
            .for_range_search();
        KNOWHERE_CONFIG_DECLARE_FIELD(enable_mmap)
            .set_default(false)
            .description("enable mmap for load index")
            .for_deserialize_from_file();
        KNOWHERE_CONFIG_DECLARE_FIELD(for_tuning).set_default(false).description("for tuning").for_search();
    }

    virtual Status
    CheckAndAdjustForSearch(std::string* err_msg) {
        return Status::success;
    }

    virtual Status
    CheckAndAdjustForRangeSearch() {
        return Status::success;
    }

    virtual inline Status
    CheckAndAdjustForBuild() {
        return Status::success;
    }
};
}  // namespace knowhere

#endif /* CONFIG_H */
