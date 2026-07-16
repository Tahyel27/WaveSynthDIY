#pragma once

#include <optional>
#include "hardware/pio.h"
#include "hardware/gpio.h"

struct PioHandler {
    pio_sm_config config;
    PIO pio = nullptr;
    uint sm = 0;
    uint offset = 0;
    const pio_program_t* program = nullptr;

    using ConfigFunc = pio_sm_config(*)(uint);

    // --- 1. CONSTRUCTORS & DESTRUCTOR (RAII & Move Semantics) ---

    // Default constructor (creates an empty, invalid handler)
    PioHandler() = default;

    // Delete copy semantics to prevent hardware resource duplication
    PioHandler(const PioHandler&) = delete;
    PioHandler& operator=(const PioHandler&) = delete;

    // Custom move constructor
    PioHandler(PioHandler&& other) noexcept 
        : config(other.config), pio(other.pio), sm(other.sm), 
          offset(other.offset), program(other.program) {
        // Nullify the source so it doesn't release the hardware on destruction
        other.pio = nullptr; 
    }

    // Custom move assignment
    PioHandler& operator=(PioHandler&& other) noexcept {
        if (this != &other) {
            release(); // Free existing resources before taking new ones
            config = other.config;
            pio = other.pio;
            sm = other.sm;
            offset = other.offset;
            program = other.program;
            other.pio = nullptr;
        }
        return *this;
    }

    // --- 2. ACQUIRE & RELEASE ---

    static std::optional<PioHandler> acquire(const pio_program_t* prog, ConfigFunc config_func) {
        PIO pio_inst;
        uint sm_idx;
        uint offset_val;

        if (!pio_claim_free_sm_and_add_program(prog, &pio_inst, &sm_idx, &offset_val)) {
            return std::nullopt; // No free state machines available
        }

        PioHandler handler;
        handler.config = config_func(offset_val);
        handler.pio = pio_inst;
        handler.sm = sm_idx;
        handler.offset = offset_val;
        handler.program = prog;

        return handler;
    }

    void release() {
        if (pio != nullptr && program != nullptr) {
            pio_sm_set_enabled(pio, sm, false);
            pio_sm_unclaim(pio, sm);
            pio_remove_program(pio, program, offset);
            pio = nullptr; // Mark as released
            program = nullptr;
        }
    }


    ~PioHandler()
    {
        release();
    }

    // --- 3. CONFIGURATION MODIFIERS ---

    void set_set_pins(uint base, uint count = 1) {
        for (uint i = 0; i < count; ++i) pio_gpio_init(pio, base + i);
        pio_sm_set_consecutive_pindirs(pio, sm, base, count, true);
        sm_config_set_set_pins(&config, base, count);
    }

    void set_in_pins(uint base, uint count = 1) {
        for (uint i = 0; i < count; ++i) pio_gpio_init(pio, base + i);
        pio_sm_set_consecutive_pindirs(pio, sm, base, count, false);
        sm_config_set_in_pins(&config, base); 
    }

    void set_out_pins(uint base, uint count = 1) {
        for (uint i = 0; i < count; ++i) pio_gpio_init(pio, base + i);
        pio_sm_set_consecutive_pindirs(pio, sm, base, count, true);
        sm_config_set_out_pins(&config, base, count);
    }

    void set_sideset_pins(uint base, uint count = 1, bool is_out = true) {
        for (uint i = 0; i < count; ++i) pio_gpio_init(pio, base + i);
        pio_sm_set_consecutive_pindirs(pio, sm, base, count, is_out);
        sm_config_set_sideset_pins(&config, base); 
    }

    void set_sideset_out_pins(uint base, uint count = 1) {
        for (uint i = 0; i < count; ++i) pio_gpio_init(pio, base + i);
        pio_sm_set_consecutive_pindirs(pio, sm, base, count, true);
    }

    void set_clkdiv_int_frac8(uint32_t div_int, uint8_t div_frac) {
        sm_config_set_clkdiv_int_frac8(&config, div_int, div_frac);
    }

    void set_in_shift(bool shift_right, bool autopush, uint push_threshold) {
        sm_config_set_in_shift(&config, shift_right, autopush, push_threshold);
    }

    void set_out_shift(bool shift_right, bool autopull, uint pull_threshold) {
        sm_config_set_out_shift(&config, shift_right, autopull, pull_threshold);
    }

    void set_wrap(uint wrap_target, uint wrap) {
        sm_config_set_wrap(&config, offset + wrap_target, offset + wrap);
    }

    // --- 4. EXECUTION & STATE ---

    void init() {
        pio_sm_init(pio, sm, offset, &config);
    }

    void set_enabled(bool enabled) {
        pio_sm_set_enabled(pio, sm, enabled);
    }

    void restart() {
        pio_sm_restart(pio, sm);
    }

    void exec(uint instr) {
        pio_sm_exec(pio, sm, instr);
    }

    // --- 5. FIFO & DMA UTILITIES ---

    void clear_fifos() {
        pio_sm_clear_fifos(pio, sm);
    }

    uint8_t get_rx_fifo_level() const {
        return pio_sm_get_rx_fifo_level(pio, sm);
    }

    uint8_t get_tx_fifo_level() const {
        return pio_sm_get_tx_fifo_level(pio, sm);
    }

    uint32_t get_blocking() const {
        return pio_sm_get_blocking(pio, sm);
    }

    void put_blocking(uint32_t data) const {
        pio_sm_put_blocking(pio, sm, data);
    }

    uint get_dreq_tx() const {
        return pio_get_dreq(pio, sm, true);
    }

    uint get_dreq_rx() const {
        return pio_get_dreq(pio, sm, false);
    }

    volatile io_wo_32 *get_tx_fifo_addr() const
    {
        return &pio->txf[sm];
    }

    volatile io_ro_32 *get_rx_fifo_addr() const
    {
        return &pio->rxf[sm];
    }
};