#pragma once

#include "common/peripheral.hpp"

namespace DMA {

class DMA : public Common::Interface {
protected:
    int channel = -1;

public:
    static inline int __get_dma_instance(DMA &dma) {
        return dma.channel; // IMPROVE: Check for valid channel?
    }

    static inline int __get_dma_instance(DMA *dma) {
        return dma->channel; // IMPROVE: Check for valid channel?
    }

#define __DMA_Trigger(instance) (1u << (::DMA::DMA::__get_dma_instance(instance))) |
/**
 * @brief Trigger a single or multiple DMA instances at the same time
 * @note Only pass DMA\::DMA class instances, compiler should error otherwise
 *
 * @param dma_interfaces vardic parameter for multiple DMA instances
 */
#define DMA_Trigger(dma_interfaces...) dma_start_channel_mask(FOR_EACH(__DMA_Trigger, dma_interfaces) 0U)
};

} // namespace DMA