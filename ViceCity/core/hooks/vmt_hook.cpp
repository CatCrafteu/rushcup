#include "core/pch.hpp"
#include "core/hooks/vmt_hook.hpp"
#include "core/memory/pattern_scanner.hpp"

namespace core::hooks {

VMTHook::VMTHook(void* instance) {
    Initialize(instance);
}

VMTHook::~VMTHook() {
    Shutdown();
}

bool VMTHook::Initialize(void* instance) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_instance) return false;
    if (!instance) return false;
    
    m_instance = instance;
    void** vtable = *reinterpret_cast<void***>(instance);
    
    // Count VTable entries
    size_t count = 0;
    while (vtable[count] && count < 512) {
        // Heuristic: check if pointer is in executable memory
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(vtable[count], &mbi, sizeof(mbi)) &&
            (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
            m_originalVTable.push_back(vtable[count]);
            count++;
        } else {
            break;
        }
    }
    
    LOG_DEBUG(Hooks, "VMTHook initialized for instance {:p}, vtable size: {}", instance, count);
    return count > 0;
}

void VMTHook::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    UnhookAll();
    m_instance = nullptr;
    m_originalVTable.clear();
    m_hooks.clear();
}

bool VMTHook::Hook(size_t index, void* newFunc) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (index >= m_originalVTable.size()) return false;
    if (!m_instance) return false;
    
    void** vtable = *reinterpret_cast<void***>(m_instance);
    DWORD oldProtect;
    
    if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
        LOG_ERROR(Hooks, "VirtualProtect failed for hook index {}", index);
        return false;
    }
    
    m_hooks[index] = vtable[index];
    vtable[index] = newFunc;
    
    VirtualProtect(&vtable[index], sizeof(void*), oldProtect, &oldProtect);
    
    LOG_DEBUG(Hooks, "Hooked index {}: {:p} -> {:p}", index, m_hooks[index], newFunc);
    return true;
}

bool VMTHook::Unhook(size_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (index >= m_originalVTable.size()) return false;
    if (!m_instance) return false;
    
    void** vtable = *reinterpret_cast<void***>(m_instance);
    auto it = m_hooks.find(index);
    if (it == m_hooks.end()) return false;
    
    DWORD oldProtect;
    if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
        return false;
    }
    
    vtable[index] = it->second;
    m_hooks.erase(it);
    
    VirtualProtect(&vtable[index], sizeof(void*), oldProtect, &oldProtect);
    
    LOG_DEBUG(Hooks, "Unhooked index {}", index);
    return true;
}

void VMTHook::UnhookAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_instance) return;
    
    void** vtable = *reinterpret_cast<void***>(m_instance);
    DWORD oldProtect;
    
    if (!VirtualProtect(vtable, m_originalVTable.size() * sizeof(void*), PAGE_READWRITE, &oldProtect)) {
        return;
    }
    
    for (size_t i = 0; i < m_originalVTable.size(); ++i) {
        vtable[i] = m_originalVTable[i];
    }
    
    VirtualProtect(vtable, m_originalVTable.size() * sizeof(void*), oldProtect, &oldProtect);
    m_hooks.clear();
    
    LOG_DEBUG(Hooks, "Unhooked all ({} hooks)", m_originalVTable.size());
}

bool VMTHook::IsHooked(size_t index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_hooks.find(index) != m_hooks.end();
}

} // namespace core::hooks