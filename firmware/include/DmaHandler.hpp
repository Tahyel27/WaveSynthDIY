#pragma once

#include <cstdint>
#include <optional>
#include "hardware/dma.h"

struct DmaHandler
{
    int channel = -1;

    // --- 1. CONSTRUCTORS & DESTRUCTOR (RAII & Move Semantics) ---

    DmaHandler() = default;

    DmaHandler(const DmaHandler &) = delete;
    DmaHandler &operator=(const DmaHandler &) = delete;

    DmaHandler(DmaHandler &&other) noexcept
        : channel(other.channel)
    {
        other.channel = -1;
    }

    DmaHandler &operator=(DmaHandler &&other) noexcept
    {
        if (this != &other)
        {
            release();
            channel = other.channel;
            other.channel = -1;
        }
        return *this;
    }

    ~DmaHandler()
    {
        release();
    }

    // --- 2. ACQUIRE & RELEASE ---

    static std::optional<DmaHandler> acquire()
    {
        int ch = dma_claim_unused_channel(false);
        if (ch == -1)
        {
            return std::nullopt;
        }

        DmaHandler handler;
        handler.channel = ch;
        return handler;
    }

    void release()
    {
        if (channel != -1)
        {
            dma_channel_abort(channel);
            dma_channel_unclaim(channel);
            channel = -1;
        }
    }

    // --- 3. CONFIGURATION ---

    // Returns a fresh default config struct for this specific channel
    dma_channel_config get_default_config() const
    {
        return dma_channel_get_default_config(channel);
    }

    // Takes the standard SDK config struct and applies it
    void configure(const dma_channel_config &config, volatile void *write_addr, const volatile void *read_addr, uint transfer_count, bool trigger)
    {
        dma_channel_configure(channel, &config, write_addr, read_addr, transfer_count, trigger);
    }

    // --- 4. EXECUTION & STATE ---

    void start() const
    {
        dma_channel_start(channel);
    }

    void abort() const
    {
        dma_channel_abort(channel);
    }

    int get_channel() const
    {
        return channel;
    }

    volatile uint32_t *get_al3_read_addr_trig_reg() const
    {
        return &dma_hw->ch[channel].al3_read_addr_trig;
    }

    // --- 5. INTERRUPT UTILITIES ---

    void set_irq0_enabled(bool enabled) const
    {
        dma_channel_set_irq0_enabled(channel, enabled);
    }

    bool get_irq0_status() const
    {
        return dma_channel_get_irq0_status(channel);
    }

    void acknowledge_irq0() const
    {
        dma_channel_acknowledge_irq0(channel);
    }

    bool check_irq0() const {
        if (dma_channel_get_irq0_status(channel)) 
        {
            dma_channel_acknowledge_irq0(channel);
            return true;
        }
        else 
        {
            return false;
        }
    }
};