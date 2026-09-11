#include <Geode/Geode.hpp>

#include "gdr.hpp"

cocos2d::CCPoint dataFromString(std::string dataString) {
    std::stringstream ss(dataString);
    std::string item;

    try {
        // The legacy format stores an unused field before X and Y.
        for (int i = 0; i < 3; i++) {
            if (!std::getline(ss, item, ',')) return { 0.f, 0.f };
            if (i == 1) {
                const float xPos = std::stof(item);
                if (!std::getline(ss, item, ',')) return { 0.f, 0.f };
                return { xPos, std::stof(item) };
            }
        }
    }
    catch (std::exception const&) {
        // Invalid imported frame-fix data should simply be ignored.
    }

    return { 0.f, 0.f };
};

std::vector<std::string> splitByChar(std::string str, char splitChar) {
    std::vector<std::string> strs;
    strs.reserve(std::count(str.begin(), str.end(), splitChar) + 1);

    size_t start = 0;
    size_t end = str.find(splitChar);
    while (end != std::string::npos) {
        strs.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(splitChar, start);
    }
    strs.emplace_back(str.substr(start));

    return strs;
}

geode::prelude::VersionInfo getVersion(std::vector<std::string> nums) {
    if (nums.size() != 3)
        return { 0, 0, 0 };

    auto major = geode::utils::numFromString<int>(nums[0]);
    auto minor = geode::utils::numFromString<int>(nums[1]);
    auto patch = geode::utils::numFromString<int>(nums[2]);

    if (major.isErr() || minor.isErr() || patch.isErr() ||
        major.unwrap() < 0 || minor.unwrap() < 0 || patch.unwrap() < 0)
        return { 0, 0, 0 };

    return {
        static_cast<size_t>(major.unwrap()),
        static_cast<size_t>(minor.unwrap()),
        static_cast<size_t>(patch.unwrap())
    };
}
