#pragma once

#include <sfdm/sfdm_config.hpp>

#include <sfdm/decode_result.hpp>
#include <sfdm/icode_reader.hpp>
#include <sfdm/image_view.hpp>

#ifdef SFDM_WITH_ZXING_DECODER
#include <sfdm/zxing_code_reader.hpp>
#endif
#ifdef SFDM_WITH_LIBDMTX_DECODER
#include <sfdm/libdmtx_code_reader.hpp>
#endif

#if defined(SFDM_WITH_ZXING_DECODER) && defined(SFDM_WITH_LIBDMTX_DECODER)
#include <sfdm/libdmtx_zxing_combined_code_reader.hpp>
#endif
