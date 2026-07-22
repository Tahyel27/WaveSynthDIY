#include <SynthCore/Components.hpp>
#include "wavetable_data.hpp"
#include <cmath>

inline int getBandIndex(float f, int wt_index)
{
    int b_index = 0;
    bool found = false;
    auto wavetable = wt_library[wt_index];
    for (int i = 0; i < wavetable.num; i++)
    {
        if (wavetable[i].f > f)
        {
            return i;
        }
    }
    return wavetable.num - 1;
}

inline Synth::float_t sampleTableLinear(const int16_t *t, float_t x)
{
    const Synth::float_t mul = 3.05185e-5;
    int i = static_cast<int>(x);
    Synth::float_t f = x - i;
    return static_cast<Synth::float_t>((1 - f) * t[i] + f * t[i + 1]) * mul;
}

static inline Synth::float_t sampleTableLinearFixed(const int16_t *t, uint32_t t10Q22)
{
    const int16_t A = t[t10Q22 >> 22];
    const int16_t B = t[(t10Q22 >> 22) + 1];
    const uint32_t max32bit = ((uint32_t)0 - 1);
    const uint32_t max16bit = max32bit >> 16;
    const float bit16tofl = 1. / static_cast<float>(max16bit);
    const float tableTo0float1 = bit16tofl * bit16tofl;

    const uint32_t angle = (max32bit >> 10) & t10Q22;

    const uint32_t inter = angle >> 6;

    const int32_t value = (uint32_t)A * (max16bit - inter) + (uint32_t)B * (inter);
    return static_cast<Synth::float_t>(value) * tableTo0float1;
}

int Synth::op_add(Instruction inst, Context &ctx)
{
    // ADD, reg: op1, reg: op2, reg: result
    float_t * out = ctx.get_scalar(inst.op3);
    *out = *ctx.get_scalar(inst.op1) + *ctx.get_scalar(inst.op2);
    return 0;
}

int Synth::op_mul(Instruction inst, Context &ctx)
{
    // MUL, reg: op1, reg: op2, reg: result
    float_t * out = ctx.get_scalar(inst.op3);
    *out = *ctx.get_scalar(inst.op1) * *ctx.get_scalar(inst.op2);
    return 0;
}

int Synth::op_add_sb(Instruction inst, Context &ctx)
{
    // ADD_SB, reg: op1, reg: op2, reg: result
    auto buf1 = ctx.get_short_buffer(inst.op1);
    auto buf2 = ctx.get_short_buffer(inst.op2);
    auto out = ctx.get_short_buffer(inst.op3);

    out.first() = buf1.first() + buf2.first();
    out.second() = buf1.second() + buf2.second();
    return 0;
}

int Synth::op_mul_sb(Instruction inst, Context &ctx)
{
    // MUL_SB, reg: op1, reg: op2, reg: result
    auto buf1 = ctx.get_short_buffer(inst.op1);
    auto buf2 = ctx.get_short_buffer(inst.op2);
    auto out = ctx.get_short_buffer(inst.op3);

    out.first() = buf1.first() * buf2.first();
    out.second() = buf1.second() * buf2.second();
    return 0;
}


int Synth::op_mix(Instruction inst, Context &ctx)
{
    //MIX, reg: buf1, reg: buf2, reg: amount1, reg: amount2, reg: out
    float_t * in_1 = ctx.get_buffer(inst.op1);
    float_t * in_2 = ctx.get_buffer(inst.op2);
    float_t * out = ctx.get_buffer(inst.op5);

    float_t a1 = *ctx.get_scalar(inst.op3);
    float_t a2 = *ctx.get_scalar(inst.op4);

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        out[i] = in_1[i] * a1 + in_2[i] * a2;
    }

    return 0;
}

int Synth::op_wtosc(Instruction inst, Context &ctx)
{
    // WTOSC, reg1: freq, reg2: phi, reg3: phasedist, reg4: wt_index, reg5: morph, reg6: out
    float_t fstart = 0.;
    float_t fend = 0.;
    if (inst.op1.type == OperandType::SHORTBUF_REG) 
    {
        auto buf = ctx.get_short_buffer(inst.op1);
        fstart = buf.first();
        fend = buf.second();
    }
    else
    {
        fstart = *ctx.get_scalar(inst.op1);
        fend = fstart;
    }

    uint32_t * phi = ctx.get_uint32(inst.op2);
    
    bool use_phasemod = false;
    if (inst.op3.type == OperandType::BUFFER_REG) use_phasemod = true;
    float_t * phimod_buf = ctx.get_buffer(inst.op3);
    uint32_t wt_index = *ctx.get_uint32(inst.op4);
    auto morph = ctx.get_short_buffer(inst.op5);
    float_t * outbuffer = ctx.get_buffer(inst.op6);

    int band_index = getBandIndex(fstart, wt_index);
    auto table = wt_library[wt_index][band_index].data;
    auto tablesize = wt_library[wt_index][band_index].length;

    const float increment = static_cast<float>(tablesize) / static_cast<float>(SPS);
    float_t tablesize_f = static_cast<float_t>(tablesize);
    const uint32_t phaseInrement_base = (1 << 22);
    const uint32_t max32bit = ((uint32_t)0 - 1);
    const float scale = 1.f / static_cast<float>((max32bit >> 10));
    const float phi_iAf = static_cast<float>(phaseInrement_base) * tablesize_f * fstart / SPS;
    const uint32_t phi_iA = static_cast<uint32_t>(phi_iAf);

    if (use_phasemod) 
    {
        const float_t dist_base = static_cast<float>(phaseInrement_base) * tablesize_f * phimod_buf[0];

        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            *phi += phi_iA;
            float_t dist = static_cast<float>(phaseInrement_base) * tablesize_f * phimod_buf[i];
            const uint32_t phiAdist = *phi + static_cast<uint32_t>(dist);

            outbuffer[i] = sampleTableLinearFixed(table, phiAdist);
        }
    }
    else 
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            *phi += phi_iA;

            outbuffer[i] = sampleTableLinearFixed(table, *phi);
        }
    }

    return 0;
}

int Synth::op_svfilt_lp(Instruction inst, Context &ctx)
{
    // SVFILT, reg1: z1, reg2: z2, reg3: signal, reg4: cutoff, reg5: q, reg6: out
    float_t & z1 = *ctx.get_scalar(inst.op1);
    float_t & z2 = *ctx.get_scalar(inst.op2);
    float_t * input = ctx.get_buffer(inst.op3);
    auto cutoff = ctx.get_short_buffer(inst.op4);
    auto q = ctx.get_short_buffer(inst.op5);
    float_t * output = ctx.get_buffer(inst.op6);

    const float_t iSPS = 1.f / static_cast<float>(SPS);
    const float_t g = (cutoff.first()) * M_PI * iSPS;
    const float_t d = 1 / (1 + 2 * q.first() * g + g * g);

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        float_t in = input[i];

        float_t g = (cutoff.next()) * M_PI * iSPS;
        float_t d = 1/(1 + 2*q.next()*g + g*g);
        float_t BP = (g * (in - z2) + z1) * d;
        float_t v1 = BP - z1;
        z1 = BP + v1;
        float_t v2 = g * BP;
        float_t LP = v2 + z2;
        z2 = LP + v2;

        output[i] = LP;
    }

    return 0;
}

int Synth::op_unison(Instruction inst, Context &ctx)
{
    //UNISON, reg: freq, reg: detune, reg: out2, reg: out3
    float_t & f = *ctx.get_scalar(inst.op1);
    float_t det = *ctx.get_scalar(inst.op2);
    *ctx.get_scalar(inst.op3) = f + f * det;
    *ctx.get_scalar(inst.op4) = f - f * det;

    return 0;
}

int Synth::op_ampl(Instruction inst, Context &ctx)
{
    //AMPL, reg: signal, reg: gain, reg: out
    float_t * in = ctx.get_buffer(inst.op1);
    float_t * out = ctx.get_buffer(inst.op3);
    if (inst.op2.type == OperandType::SCALAR_REG || inst.op2.type == OperandType::EXT_REG || inst.op2.type == OperandType::IMMEDIATE) 
    {
        float_t gain = *ctx.get_scalar(inst.op2);
        for (int i = 0; i < CHUNK_SIZE; i++)
        {
            out[i] = gain * in[i];
        }
    }
    else if (inst.op2.type == OperandType::SHORTBUF_REG) 
    {
        auto gain = ctx.get_short_buffer(inst.op2);
        for (int i = 0; i < CHUNK_SIZE; i++)
        {
            out[i] = gain.next() * in[i];
        }
    }
    else if (inst.op2.type == OperandType::BUFFER_REG)
    {
        float_t * gain = ctx.get_buffer(inst.op2);
        for (int i = 0; i < CHUNK_SIZE; i++)
        {
            out[i] = gain[i] * in[i];
        }
    }

    return 0;
}

int Synth::op_adsr(Instruction inst, Context &ctx)
{
    // ADSR, reg1: signal, reg2: state, reg3: time, reg4: attack, reg5: decay, reg6: sustain, reg7: release, reg8: out
    float_t gate = 0.0f;
    if (inst.op1.type == OperandType::SHORTBUF_REG) gate = ctx.get_short_buffer(inst.op1).first();
    else if (inst.op1.type == OperandType::BUFFER_REG) gate = ctx.get_buffer(inst.op1)[0];
    else gate = *ctx.get_scalar(inst.op1);

    uint32_t *state_reg = ctx.get_uint32(inst.op2);
    float_t *env_val = ctx.get_scalar(inst.op3); // 'time' register stores current amplitude
    float_t attack = *ctx.get_scalar(inst.op4);
    float_t decay = *ctx.get_scalar(inst.op5);
    float_t sustain = std::clamp(*ctx.get_scalar(inst.op6), 0.0f, 1.0f);
    float_t release = *ctx.get_scalar(inst.op7);
    float_t *outbuffer = ctx.get_buffer(inst.op8);

    ADSRState state = static_cast<ADSRState>(*state_reg);

    // Gate logic
    if (gate > 0.5f && (state == ADSRState::IDLE || state == ADSRState::RELEASE))
    {
        state = ADSRState::ATTACK;
    }
    else if (gate <= 0.5f && state != ADSRState::IDLE && state != ADSRState::RELEASE)
    {
        state = ADSRState::RELEASE;
    }

    float_t current_val = *env_val;
    float_t attack_rate = dt / (attack + 1e-5f);
    float_t decay_rate = (1.0f - sustain) * dt / (decay + 1e-5f);
    float_t release_rate = dt / (release + 1e-5f);

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        if (state == ADSRState::ATTACK)
        {
            current_val += attack_rate;
            if (current_val >= 1.0f)
            {
                current_val = 1.0f;
                state = ADSRState::DECAY;
            }
        }
        else if (state == ADSRState::DECAY)
        {
            current_val -= decay_rate;
            if (current_val <= sustain)
            {
                current_val = sustain;
                state = ADSRState::SUSTAIN;
            }
        }
        else if (state == ADSRState::SUSTAIN)
        {
            current_val = sustain;
        }
        else if (state == ADSRState::RELEASE)
        {
            current_val -= release_rate;
            if (current_val <= 0.0f)
            {
                current_val = 0.0f;
                state = ADSRState::IDLE;
            }
        }
        else // IDLE
        {
            current_val = 0.0f;
        }

        outbuffer[i] = current_val;
    }

    *env_val = current_val;
    *state_reg = static_cast<uint32_t>(state);

    return 0;
}

int Synth::op_sineosc(Instruction inst, Context &ctx)
{
    // SINEOSC, reg1: freq, reg2: phi, reg3: phasedist, reg4: out
    float_t fstart = 0.;
    if (inst.op1.type == OperandType::SHORTBUF_REG) 
    {
        fstart = ctx.get_short_buffer(inst.op1).first();
    }
    else
    {
        fstart = *ctx.get_scalar(inst.op1);
    }

    uint32_t * phi = ctx.get_uint32(inst.op2);
    
    bool use_phasemod = false;
    if (inst.op3.type == OperandType::BUFFER_REG) use_phasemod = true;
    float_t * phimod_buf = ctx.get_buffer(inst.op3);
    float_t * outbuffer = ctx.get_buffer(inst.op4);

    auto table = wt_library[0][0].data;
    auto tablesize = wt_library[0][0].length;

    const uint32_t phaseInrement_base = (1 << 22);
    float_t tablesize_f = static_cast<float_t>(tablesize);
    const float phi_iAf = static_cast<float>(phaseInrement_base) * tablesize_f * fstart / SPS;
    const uint32_t phaseIncrement = static_cast<uint32_t>(phi_iAf);

    if (use_phasemod) 
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            *phi += phaseIncrement;
            float_t dist = static_cast<float>(phaseInrement_base) * tablesize_f * phimod_buf[i];
            const uint32_t phiAdist = *phi + static_cast<uint32_t>(dist);

            outbuffer[i] = sampleTableLinearFixed(table, phiAdist) * 4.0f;
        }
    }
    else 
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            *phi += phaseIncrement;
            outbuffer[i] = sampleTableLinearFixed(table, *phi) * 4.0f;
        }
    }

    return 0;
}

int Synth::op_lfosine(Instruction inst, Context &ctx)
{
    float_t freq_start = 0.0f;
    float_t freq_end = 0.0f;
    if (inst.op1.type == OperandType::SHORTBUF_REG) {
        freq_start = ctx.get_short_buffer(inst.op1).first();
        freq_end = ctx.get_short_buffer(inst.op1).second();
    } else {
        freq_start = *ctx.get_scalar(inst.op1);
        freq_end = freq_start;
    }

    uint32_t *phi = ctx.get_uint32(inst.op2);
    
    float_t dist_start = 0.0f;
    float_t dist_end = 0.0f;
    if (inst.op3.type == OperandType::SHORTBUF_REG) {
        dist_start = ctx.get_short_buffer(inst.op3).first();
        dist_end = ctx.get_short_buffer(inst.op3).second();
    } else {
        dist_start = *ctx.get_scalar(inst.op3);
        dist_end = dist_start;
    }

    auto outbuffer = ctx.get_short_buffer(inst.op4);
    auto table = wt_library[0][0].data;

    auto calc_sine = [&](uint32_t phase, float_t dist) -> float_t {
        // 4294967296.0f is 2^32. This maps the 0.0-1.0 phase distortion value onto the full 32-bit unsigned integer range.
        uint32_t dist_offset = static_cast<uint32_t>(dist * 4294967296.0f);
        return sampleTableLinearFixed(table, phase + dist_offset) * 4.0f;
    };

    outbuffer.first() = calc_sine(*phi, dist_start);

    float_t avg_freq = (freq_start + freq_end) * 0.5f;
    // 4294967296.0f is 2^32. This converts the frequency (Hz) into a 32-bit phase increment per second.
    uint32_t chunk_increment = static_cast<uint32_t>((avg_freq / SPS) * 4294967296.0f * CHUNK_SIZE);

    *phi += chunk_increment;
    outbuffer.second() = calc_sine(*phi, dist_end);

    return 0;
}

int Synth::op_lfotri(Instruction inst, Context &ctx)
{
    float_t freq_start = 0.0f;
    float_t freq_end = 0.0f;
    if (inst.op1.type == OperandType::SHORTBUF_REG) {
        freq_start = ctx.get_short_buffer(inst.op1).first();
        freq_end = ctx.get_short_buffer(inst.op1).second();
    } else {
        freq_start = *ctx.get_scalar(inst.op1);
        freq_end = freq_start;
    }

    uint32_t *phi = ctx.get_uint32(inst.op2);
    
    float_t dist_start = 0.0f;
    float_t dist_end = 0.0f;
    if (inst.op3.type == OperandType::SHORTBUF_REG) {
        dist_start = ctx.get_short_buffer(inst.op3).first();
        dist_end = ctx.get_short_buffer(inst.op3).second();
    } else {
        dist_start = *ctx.get_scalar(inst.op3);
        dist_end = dist_start;
    }

    auto outbuffer = ctx.get_short_buffer(inst.op4);

    auto calc_tri = [](uint32_t phase, float_t dist) -> float_t {
        // 4294967296.0f is 2^32. This maps the 0.0-1.0 phase distortion value onto the 32-bit phase.
        uint32_t p = phase + static_cast<uint32_t>(dist * 4294967296.0f);
        
        // 0x80000000 is 2^31 (the midpoint of a 32-bit integer). This splits the triangle wave into its rising and falling halves.
        if (p < 0x80000000) {
            // 1073741824.0f is 2^30. Dividing by this maps the first half of the phase (0 to 2^31) into a 0.0 to 2.0 range.
            // Subtracting 1.0 shifts the output down to range from -1.0 to 1.0 (the rising edge).
            return (static_cast<float_t>(p) / 1073741824.0f) - 1.0f;
        } else {
            // Dividing by 2^30 maps the second half of the phase (2^31 to 2^32) into a 2.0 to 4.0 range.
            // Subtracting this from 3.0 flips the slope, yielding a 1.0 to -1.0 range (the falling edge).
            return 3.0f - (static_cast<float_t>(p) / 1073741824.0f);
        }
    };

    outbuffer.first() = calc_tri(*phi, dist_start);

    float_t avg_freq = (freq_start + freq_end) * 0.5f;
    // 4294967296.0f is 2^32. Converts the frequency (Hz) into a 32-bit phase increment per second.
    uint32_t chunk_increment = static_cast<uint32_t>((avg_freq / SPS) * 4294967296.0f * CHUNK_SIZE);

    *phi += chunk_increment;
    outbuffer.second() = calc_tri(*phi, dist_end);

    return 0;
}

int Synth::op_lfosaw(Instruction inst, Context &ctx)
{
    float_t freq_start = 0.0f;
    float_t freq_end = 0.0f;
    if (inst.op1.type == OperandType::SHORTBUF_REG) {
        freq_start = ctx.get_short_buffer(inst.op1).first();
        freq_end = ctx.get_short_buffer(inst.op1).second();
    } else {
        freq_start = *ctx.get_scalar(inst.op1);
        freq_end = freq_start;
    }

    uint32_t *phi = ctx.get_uint32(inst.op2);
    
    float_t dist_start = 0.0f;
    float_t dist_end = 0.0f;
    if (inst.op3.type == OperandType::SHORTBUF_REG) {
        dist_start = ctx.get_short_buffer(inst.op3).first();
        dist_end = ctx.get_short_buffer(inst.op3).second();
    } else {
        dist_start = *ctx.get_scalar(inst.op3);
        dist_end = dist_start;
    }

    auto outbuffer = ctx.get_short_buffer(inst.op4);

    auto calc_saw = [](uint32_t phase, float_t dist) -> float_t {
        // 4294967296.0f is 2^32. This maps the 0.0-1.0 phase distortion value onto the 32-bit phase.
        uint32_t p = phase + static_cast<uint32_t>(dist * 4294967296.0f);
        
        // 2147483648.0f is 2^31. Dividing by this maps the full 0 to 2^32 phase range into a 0.0 to 2.0 float range.
        // Subtracting 1.0 shifts the output down to range from -1.0 to 1.0 (a pure sawtooth).
        return (static_cast<float_t>(p) / 2147483648.0f) - 1.0f;
    };

    outbuffer.first() = calc_saw(*phi, dist_start);

    float_t avg_freq = (freq_start + freq_end) * 0.5f;
    // 4294967296.0f is 2^32. Converts the frequency (Hz) into a 32-bit phase increment per second.
    uint32_t chunk_increment = static_cast<uint32_t>((avg_freq / SPS) * 4294967296.0f * CHUNK_SIZE);

    *phi += chunk_increment;
    outbuffer.second() = calc_saw(*phi, dist_end);

    return 0;
}