#include "core/pch.hpp"
#include "core/memory/pattern_scanner.hpp"

namespace core::memory {

bool PatternCompare(const uint8_t* data, const uint8_t* pattern, const char* mask) {
    for (; *mask; ++mask, ++data, ++pattern) {
        if (*mask == 'x' && *data != *pattern) {
            return false;
        }
    }
    return true;
}

std::pair<uintptr_t, size_t> GetModuleInfo(const char* moduleName) {
    HMODULE module = GetModuleHandleA(moduleName);
    if (!module) return {0, 0};
    
    MODULEINFO info{};
    if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info))) {
        return {0, 0};
    }
    
    return {reinterpret_cast<uintptr_t>(module), info.SizeOfImage};
}

std::optional<uintptr_t> PatternScanner::Scan(const char* moduleName, const char* pattern, const char* mask) {
    auto [base, size] = GetModuleInfo(moduleName);
    if (!base || !size) return std::nullopt;
    return Scan(base, size, pattern, mask);
}

std::optional<uintptr_t> PatternScanner::Scan(uintptr_t base, size_t size, const char* pattern, const char* mask) {
    if (!base || !size || !pattern) return std::nullopt;
    
    const char* defaultMask = mask ? mask : pattern;
    const uint8_t* patternBytes = reinterpret_cast<const uint8_t*>(pattern);
    const uint8_t* scanEnd = reinterpret_cast<uint8_t*>(base + size);
    const size_t patternLen = mask ? strlen(mask) : strlen(pattern);
    
    for (uintptr_t addr = base; addr < base + size - patternLen; ++addr) {
        if (PatternCompare(reinterpret_cast<const uint8_t*>(addr), patternBytes, defaultMask)) {
            return addr;
        }
    }
    
    return std::nullopt;
}

std::vector<uintptr_t> PatternScanner::ScanAll(const char* moduleName, const char* pattern, const char* mask) {
    std::vector<uintptr_t> results;
    auto [base, size] = GetModuleInfo(moduleName);
    if (!base || !size) return results;
    
    const char* defaultMask = mask ? mask : pattern;
    const uint8_t* patternBytes = reinterpret_cast<const uint8_t*>(pattern);
    const size_t patternLen = mask ? strlen(mask) : strlen(pattern);
    
    for (uintptr_t addr = base; addr < base + size - patternLen; ++addr) {
        if (PatternCompare(reinterpret_cast<const uint8_t*>(addr), patternBytes, defaultMask)) {
            results.push_back(addr);
        }
    }
    
    return results;
}

std::optional<uintptr_t> PatternScanner::ScanIDA(const char* moduleName, const char* idaPattern) {
    auto [base, size] = GetModuleInfo(moduleName);
    if (!base || !size) return std::nullopt;
    return ScanIDA(base, size, idaPattern);
}

std::optional<uintptr_t> PatternScanner::ScanIDA(uintptr_t base, size_t size, const char* idaPattern) {
    if (!base || !size || !idaPattern) return std::nullopt;
    
    // Convert IDA pattern to bytes + mask
    std::vector<uint8_t> patternBytes;
    std::string mask;
    
    const char* p = idaPattern;
    while (*p) {
        if (*p == ' ') {
            ++p;
            continue;
        }
        
        if (*p == '?') {
            patternBytes.push_back(0);
            mask.push_back('?');
            ++p;
            if (*p == '?') ++p; // Skip second ?
        } else {
            char byteStr[3] = {p[0], p[1], 0};
            patternBytes.push_back(static_cast<uint8_t>(strtoul(byteStr, nullptr, 16)));
            mask.push_back('x');
            p += 2;
        }
    }
    
    if (patternBytes.empty()) return std::nullopt;
    
    const uint8_t* scanEnd = reinterpret_cast<uint8_t*>(base + size);
    const size_t patternLen = patternBytes.size();
    
    for (uintptr_t addr = base; addr < base + size - patternLen; ++addr) {
        if (PatternCompare(reinterpret_cast<const uint8_t*>(addr), patternBytes.data(), mask.c_str())) {
            return addr;
        }
    }
    
    return std::nullopt;
}

} // namespace core::memory