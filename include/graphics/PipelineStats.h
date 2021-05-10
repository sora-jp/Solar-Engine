#pragma once
#include <cstdint>

struct PipelineStats
{
    uint64_t inputVertices = 0;
    uint64_t inputPrimitives = 0;
    uint64_t gsPrimitives = 0;
    uint64_t clippingInvocations = 0;
    uint64_t clippingPrimitives = 0;
    uint64_t vsInvocations = 0;
    uint64_t gsInvocations = 0;
    uint64_t psInvocations = 0;
    uint64_t hsInvocations = 0;
    uint64_t dsInvocations = 0;
    uint64_t csInvocations = 0;
};