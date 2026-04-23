/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-04-23T14:25:51-0400
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */


#include "network.h"
#include "network_data.h"

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_math_helpers.h"

#include "core_common.h"
#include "core_convert.h"

#include "layers.h"



#undef AI_NET_OBJ_INSTANCE
#define AI_NET_OBJ_INSTANCE g_network
 
#undef AI_NETWORK_MODEL_SIGNATURE
#define AI_NETWORK_MODEL_SIGNATURE     "0xd79236cb8c50d13c5eefc957685feec0"

#ifndef AI_TOOLS_REVISION_ID
#define AI_TOOLS_REVISION_ID     ""
#endif

#undef AI_TOOLS_DATE_TIME
#define AI_TOOLS_DATE_TIME   "2026-04-23T14:25:51-0400"

#undef AI_TOOLS_COMPILE_TIME
#define AI_TOOLS_COMPILE_TIME    __DATE__ " " __TIME__

#undef AI_NETWORK_N_BATCHES
#define AI_NETWORK_N_BATCHES         (1)

static ai_ptr g_network_activations_map[1] = AI_C_ARRAY_INIT;
static ai_ptr g_network_weights_map[1] = AI_C_ARRAY_INIT;



/**  Array declarations section  **********************************************/
/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  onnxGemm_0_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 256, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  _fc1_Gemm_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Sub_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Pow_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Sqrt_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Div_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  _Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25600, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  _fc2_Gemm_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Sub_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Pow_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_Mul_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Sqrt_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Div_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  _Relu_1_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 19200, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  _mu_Gemm_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  node_38_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 128, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Constant_output_0_3D_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Constant_output_0_3D_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  _fc1_Gemm_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1600, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  _fc1_Gemm_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 400, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_scale_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Mul_output_0_scale_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 400, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  _bn1_Mul_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 400, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  _fc2_Gemm_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120000, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  _fc2_Gemm_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 300, AI_STATIC)

/* Array#36 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_scale_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1, AI_STATIC)

/* Array#37 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Mul_output_0_scale_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 300, AI_STATIC)

/* Array#38 */
AI_ARRAY_OBJ_DECLARE(
  _bn2_Mul_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 300, AI_STATIC)

/* Array#39 */
AI_ARRAY_OBJ_DECLARE(
  _mu_Gemm_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 600, AI_STATIC)

/* Array#40 */
AI_ARRAY_OBJ_DECLARE(
  _mu_Gemm_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2, AI_STATIC)

/**  Tensor declarations section  *********************************************/
/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  _Relu_1_output_0_output, AI_STATIC,
  0, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_Relu_1_output_0_output_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  _Relu_output_0_output, AI_STATIC,
  1, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_Relu_output_0_output_array, NULL)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Constant_output_0_3D, AI_STATIC,
  2, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_Constant_output_0_3D_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Div_output_0_output, AI_STATIC,
  3, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Div_output_0_output_array, NULL)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Mul_output_0_bias, AI_STATIC,
  4, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Mul_output_0_bias_array, NULL)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Mul_output_0_output, AI_STATIC,
  5, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Mul_output_0_output_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Mul_output_0_scale, AI_STATIC,
  6, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Mul_output_0_scale_array, NULL)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Pow_output_0_output, AI_STATIC,
  7, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Pow_output_0_output_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_bias, AI_STATIC,
  8, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_1_output_0_Mul_bias_array, NULL)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_output, AI_STATIC,
  9, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_1_output_0_Mul_output_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_output, AI_STATIC,
  10, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_1_output_0_output_array, NULL)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_bias, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_output_0_Mul_bias_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_output, AI_STATIC,
  12, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_output_0_Mul_output_array, NULL)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_scale, AI_STATIC,
  13, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_output_0_Mul_scale_array, NULL)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_output, AI_STATIC,
  14, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_ReduceMean_output_0_output_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Sqrt_output_0_output, AI_STATIC,
  15, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn1_Sqrt_output_0_output_array, NULL)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  _bn1_Sub_output_0_output, AI_STATIC,
  16, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_bn1_Sub_output_0_output_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Constant_output_0_3D, AI_STATIC,
  17, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_Constant_output_0_3D_array, NULL)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Div_output_0_output, AI_STATIC,
  18, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Div_output_0_output_array, NULL)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Mul_output_0_bias, AI_STATIC,
  19, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Mul_output_0_bias_array, NULL)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Mul_output_0_output, AI_STATIC,
  20, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Mul_output_0_output_array, NULL)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Mul_output_0_scale, AI_STATIC,
  21, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Mul_output_0_scale_array, NULL)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Pow_output_0_output, AI_STATIC,
  22, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Pow_output_0_output_array, NULL)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_Mul_output, AI_STATIC,
  23, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_ReduceMean_1_output_0_Mul_output_array, NULL)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_output, AI_STATIC,
  24, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_ReduceMean_1_output_0_output_array, NULL)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_output, AI_STATIC,
  25, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_ReduceMean_output_0_Mul_output_array, NULL)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_scale, AI_STATIC,
  26, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_ReduceMean_output_0_Mul_scale_array, NULL)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_output, AI_STATIC,
  27, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_ReduceMean_output_0_output_array, NULL)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Sqrt_output_0_output, AI_STATIC,
  28, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &_bn2_Sqrt_output_0_output_array, NULL)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  _bn2_Sub_output_0_output, AI_STATIC,
  29, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_bn2_Sub_output_0_output_array, NULL)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  _fc1_Gemm_output_0_bias, AI_STATIC,
  30, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_fc1_Gemm_output_0_bias_array, NULL)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  _fc1_Gemm_output_0_output, AI_STATIC,
  31, 0x0,
  AI_SHAPE_INIT(4, 1, 400, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1600, 1600),
  1, &_fc1_Gemm_output_0_output_array, NULL)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  _fc1_Gemm_output_0_weights, AI_STATIC,
  32, 0x0,
  AI_SHAPE_INIT(4, 4, 400, 1, 1), AI_STRIDE_INIT(4, 4, 16, 6400, 6400),
  1, &_fc1_Gemm_output_0_weights_array, NULL)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  _fc2_Gemm_output_0_bias, AI_STATIC,
  33, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_fc2_Gemm_output_0_bias_array, NULL)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  _fc2_Gemm_output_0_output, AI_STATIC,
  34, 0x0,
  AI_SHAPE_INIT(4, 1, 300, 1, 64), AI_STRIDE_INIT(4, 4, 4, 1200, 1200),
  1, &_fc2_Gemm_output_0_output_array, NULL)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  _fc2_Gemm_output_0_weights, AI_STATIC,
  35, 0x0,
  AI_SHAPE_INIT(4, 400, 300, 1, 1), AI_STRIDE_INIT(4, 4, 1600, 480000, 480000),
  1, &_fc2_Gemm_output_0_weights_array, NULL)

/* Tensor #36 */
AI_TENSOR_OBJ_DECLARE(
  _mu_Gemm_output_0_bias, AI_STATIC,
  36, 0x0,
  AI_SHAPE_INIT(4, 1, 2, 1, 1), AI_STRIDE_INIT(4, 4, 4, 8, 8),
  1, &_mu_Gemm_output_0_bias_array, NULL)

/* Tensor #37 */
AI_TENSOR_OBJ_DECLARE(
  _mu_Gemm_output_0_output, AI_STATIC,
  37, 0x0,
  AI_SHAPE_INIT(4, 1, 2, 1, 64), AI_STRIDE_INIT(4, 4, 4, 8, 8),
  1, &_mu_Gemm_output_0_output_array, NULL)

/* Tensor #38 */
AI_TENSOR_OBJ_DECLARE(
  _mu_Gemm_output_0_weights, AI_STATIC,
  38, 0x0,
  AI_SHAPE_INIT(4, 300, 2, 1, 1), AI_STRIDE_INIT(4, 4, 1200, 2400, 2400),
  1, &_mu_Gemm_output_0_weights_array, NULL)

/* Tensor #39 */
AI_TENSOR_OBJ_DECLARE(
  node_38_output, AI_STATIC,
  39, 0x0,
  AI_SHAPE_INIT(4, 1, 2, 1, 64), AI_STRIDE_INIT(4, 4, 4, 8, 8),
  1, &node_38_output_array, NULL)

/* Tensor #40 */
AI_TENSOR_OBJ_DECLARE(
  onnxGemm_0_output, AI_STATIC,
  40, 0x0,
  AI_SHAPE_INIT(4, 1, 4, 1, 64), AI_STRIDE_INIT(4, 4, 4, 16, 16),
  1, &onnxGemm_0_output_array, NULL)



/**  Layer declarations section  **********************************************/


AI_TENSOR_CHAIN_OBJ_DECLARE(
  node_38_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_mu_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &node_38_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  node_38_layer, 28,
  NL_TYPE, 0x0, NULL,
  nl, forward_tanh,
  &node_38_chain,
  NULL, &node_38_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _mu_Gemm_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_Relu_1_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_mu_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_mu_Gemm_output_0_weights, &_mu_Gemm_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _mu_Gemm_output_0_layer, 27,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &_mu_Gemm_output_0_chain,
  NULL, &node_38_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _Relu_1_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_Relu_1_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _Relu_1_output_0_layer, 26,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_Relu_1_output_0_chain,
  NULL, &_mu_Gemm_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn2_Mul_output_0_scale, &_bn2_Mul_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_Mul_output_0_layer, 25,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn2_Mul_output_0_chain,
  NULL, &_Relu_1_output_0_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_Div_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn2_Sub_output_0_output, &_bn2_Sqrt_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_Div_output_0_layer, 23,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn2_Div_output_0_chain,
  NULL, &_bn2_Mul_output_0_layer, AI_STATIC, 
  .operation = ai_div_f32, 
  .buffer_operation = ai_div_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_Sqrt_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_1_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Sqrt_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_Sqrt_output_0_layer, 22,
  NL_TYPE, 0x0, NULL,
  nl, forward_sqrt,
  &_bn2_Sqrt_output_0_chain,
  NULL, &_bn2_Div_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_Mul_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_1_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_1_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn2_ReduceMean_output_0_Mul_scale, &_bn1_ReduceMean_1_output_0_Mul_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_Mul_layer, 21,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn2_ReduceMean_1_output_0_Mul_chain,
  NULL, &_bn2_Sqrt_output_0_layer, AI_STATIC, 
)


AI_STATIC_CONST ai_float _bn2_ReduceMean_1_output_0_neutral_value_data[] = { 0.0f };
AI_ARRAY_OBJ_DECLARE(
    _bn2_ReduceMean_1_output_0_neutral_value, AI_ARRAY_FORMAT_FLOAT,
    _bn2_ReduceMean_1_output_0_neutral_value_data, _bn2_ReduceMean_1_output_0_neutral_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Pow_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_1_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_ReduceMean_1_output_0_layer, 21,
  REDUCE_TYPE, 0x0, NULL,
  reduce, forward_reduce,
  &_bn2_ReduceMean_1_output_0_chain,
  NULL, &_bn2_ReduceMean_1_output_0_Mul_layer, AI_STATIC, 
  .operation = ai_sum, 
  .neutral_value = &_bn2_ReduceMean_1_output_0_neutral_value, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_Pow_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn2_Sub_output_0_output, &_bn2_Constant_output_0_3D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Pow_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_Pow_output_0_layer, 18,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn2_Pow_output_0_chain,
  NULL, &_bn2_ReduceMean_1_output_0_layer, AI_STATIC, 
  .operation = ai_pow, 
  .buffer_operation = ai_pow_buffer, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_Sub_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_fc2_Gemm_output_0_output, &_bn2_ReduceMean_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_Sub_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_Sub_output_0_layer, 16,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn2_Sub_output_0_chain,
  NULL, &_bn2_Pow_output_0_layer, AI_STATIC, 
  .operation = ai_sub_f32, 
  .buffer_operation = ai_sub_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn2_ReduceMean_output_0_Mul_scale, &_bn1_ReduceMean_output_0_Mul_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_Mul_layer, 15,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn2_ReduceMean_output_0_Mul_chain,
  NULL, &_bn2_Sub_output_0_layer, AI_STATIC, 
)


AI_STATIC_CONST ai_float _bn2_ReduceMean_output_0_neutral_value_data[] = { 0.0f };
AI_ARRAY_OBJ_DECLARE(
    _bn2_ReduceMean_output_0_neutral_value, AI_ARRAY_FORMAT_FLOAT,
    _bn2_ReduceMean_output_0_neutral_value_data, _bn2_ReduceMean_output_0_neutral_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_fc2_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn2_ReduceMean_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn2_ReduceMean_output_0_layer, 15,
  REDUCE_TYPE, 0x0, NULL,
  reduce, forward_reduce,
  &_bn2_ReduceMean_output_0_chain,
  NULL, &_bn2_ReduceMean_output_0_Mul_layer, AI_STATIC, 
  .operation = ai_sum, 
  .neutral_value = &_bn2_ReduceMean_output_0_neutral_value, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _fc2_Gemm_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_fc2_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_fc2_Gemm_output_0_weights, &_fc2_Gemm_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _fc2_Gemm_output_0_layer, 14,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &_fc2_Gemm_output_0_chain,
  NULL, &_bn2_ReduceMean_output_0_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _Relu_output_0_layer, 13,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_Relu_output_0_chain,
  NULL, &_fc2_Gemm_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn1_Mul_output_0_scale, &_bn1_Mul_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_Mul_output_0_layer, 12,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn1_Mul_output_0_chain,
  NULL, &_Relu_output_0_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_Div_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn1_Sub_output_0_output, &_bn1_Sqrt_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_Div_output_0_layer, 10,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn1_Div_output_0_chain,
  NULL, &_bn1_Mul_output_0_layer, AI_STATIC, 
  .operation = ai_div_f32, 
  .buffer_operation = ai_div_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_Sqrt_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_1_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Sqrt_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_Sqrt_output_0_layer, 9,
  NL_TYPE, 0x0, NULL,
  nl, forward_sqrt,
  &_bn1_Sqrt_output_0_chain,
  NULL, &_bn1_Div_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_1_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_1_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn1_ReduceMean_output_0_Mul_scale, &_bn1_ReduceMean_1_output_0_Mul_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_Mul_layer, 8,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn1_ReduceMean_1_output_0_Mul_chain,
  NULL, &_bn1_Sqrt_output_0_layer, AI_STATIC, 
)


AI_STATIC_CONST ai_float _bn1_ReduceMean_1_output_0_neutral_value_data[] = { 0.0f };
AI_ARRAY_OBJ_DECLARE(
    _bn1_ReduceMean_1_output_0_neutral_value, AI_ARRAY_FORMAT_FLOAT,
    _bn1_ReduceMean_1_output_0_neutral_value_data, _bn1_ReduceMean_1_output_0_neutral_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Pow_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_1_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_ReduceMean_1_output_0_layer, 8,
  REDUCE_TYPE, 0x0, NULL,
  reduce, forward_reduce,
  &_bn1_ReduceMean_1_output_0_chain,
  NULL, &_bn1_ReduceMean_1_output_0_Mul_layer, AI_STATIC, 
  .operation = ai_sum, 
  .neutral_value = &_bn1_ReduceMean_1_output_0_neutral_value, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_Pow_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn1_Sub_output_0_output, &_bn1_Constant_output_0_3D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Pow_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_Pow_output_0_layer, 5,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn1_Pow_output_0_chain,
  NULL, &_bn1_ReduceMean_1_output_0_layer, AI_STATIC, 
  .operation = ai_pow, 
  .buffer_operation = ai_pow_buffer, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_Sub_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_fc1_Gemm_output_0_output, &_bn1_ReduceMean_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_Sub_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_Sub_output_0_layer, 3,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_bn1_Sub_output_0_chain,
  NULL, &_bn1_Pow_output_0_layer, AI_STATIC, 
  .operation = ai_sub_f32, 
  .buffer_operation = ai_sub_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_output_0_Mul_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_bn1_ReduceMean_output_0_Mul_scale, &_bn1_ReduceMean_output_0_Mul_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_Mul_layer, 2,
  BN_TYPE, 0x0, NULL,
  bn, forward_bn,
  &_bn1_ReduceMean_output_0_Mul_chain,
  NULL, &_bn1_Sub_output_0_layer, AI_STATIC, 
)


AI_STATIC_CONST ai_float _bn1_ReduceMean_output_0_neutral_value_data[] = { 0.0f };
AI_ARRAY_OBJ_DECLARE(
    _bn1_ReduceMean_output_0_neutral_value, AI_ARRAY_FORMAT_FLOAT,
    _bn1_ReduceMean_output_0_neutral_value_data, _bn1_ReduceMean_output_0_neutral_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_fc1_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_bn1_ReduceMean_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _bn1_ReduceMean_output_0_layer, 2,
  REDUCE_TYPE, 0x0, NULL,
  reduce, forward_reduce,
  &_bn1_ReduceMean_output_0_chain,
  NULL, &_bn1_ReduceMean_output_0_Mul_layer, AI_STATIC, 
  .operation = ai_sum, 
  .neutral_value = &_bn1_ReduceMean_output_0_neutral_value, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _fc1_Gemm_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &onnxGemm_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_fc1_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_fc1_Gemm_output_0_weights, &_fc1_Gemm_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _fc1_Gemm_output_0_layer, 1,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &_fc1_Gemm_output_0_chain,
  NULL, &_bn1_ReduceMean_output_0_layer, AI_STATIC, 
)


#if (AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 497232, 1, 1),
    497232, NULL, NULL),
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 205056, 1, 1),
    205056, NULL, NULL),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &onnxGemm_0_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &node_38_output),
  &_fc1_Gemm_output_0_layer, 0xf394ddd2, NULL)

#else

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 497232, 1, 1),
      497232, NULL, NULL)
  ),
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 205056, 1, 1),
      205056, NULL, NULL)
  ),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &onnxGemm_0_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &node_38_output),
  &_fc1_Gemm_output_0_layer, 0xf394ddd2, NULL)

#endif	/*(AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)*/



/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_activations(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_activations_map(g_network_activations_map, 1, params)) {
    /* Updating activations (byte) offsets */
    
    onnxGemm_0_output_array.data = AI_PTR(g_network_activations_map[0] + 101376);
    onnxGemm_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 101376);
    _fc1_Gemm_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 102400);
    _fc1_Gemm_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 102400);
    _bn1_ReduceMean_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 101376);
    _bn1_ReduceMean_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 101376);
    _bn1_ReduceMean_output_0_Mul_output_array.data = AI_PTR(g_network_activations_map[0] + 101632);
    _bn1_ReduceMean_output_0_Mul_output_array.data_start = AI_PTR(g_network_activations_map[0] + 101632);
    _bn1_Sub_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 102400);
    _bn1_Sub_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 102400);
    _bn1_Pow_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _bn1_Pow_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _bn1_ReduceMean_1_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 204800);
    _bn1_ReduceMean_1_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 204800);
    _bn1_ReduceMean_1_output_0_Mul_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _bn1_ReduceMean_1_output_0_Mul_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _bn1_Sqrt_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 256);
    _bn1_Sqrt_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 256);
    _bn1_Div_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 102400);
    _bn1_Div_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 102400);
    _bn1_Mul_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _bn1_Mul_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 102400);
    _Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 102400);
    _fc2_Gemm_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _fc2_Gemm_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _bn2_ReduceMean_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 76800);
    _bn2_ReduceMean_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 76800);
    _bn2_ReduceMean_output_0_Mul_output_array.data = AI_PTR(g_network_activations_map[0] + 77056);
    _bn2_ReduceMean_output_0_Mul_output_array.data_start = AI_PTR(g_network_activations_map[0] + 77056);
    _bn2_Sub_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 77312);
    _bn2_Sub_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 77312);
    _bn2_Pow_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _bn2_Pow_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _bn2_ReduceMean_1_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 76800);
    _bn2_ReduceMean_1_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 76800);
    _bn2_ReduceMean_1_output_0_Mul_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _bn2_ReduceMean_1_output_0_Mul_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _bn2_Sqrt_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 256);
    _bn2_Sqrt_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 256);
    _bn2_Div_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 512);
    _bn2_Div_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 512);
    _bn2_Mul_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 77312);
    _bn2_Mul_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 77312);
    _Relu_1_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _Relu_1_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _mu_Gemm_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 76800);
    _mu_Gemm_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 76800);
    node_38_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    node_38_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_ACTIVATIONS);
  return false;
}




/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_weights(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_weights_map(g_network_weights_map, 1, params)) {
    /* Updating weights (byte) offsets */
    
    _bn1_Constant_output_0_3D_array.format |= AI_FMT_FLAG_CONST;
    _bn1_Constant_output_0_3D_array.data = AI_PTR(g_network_weights_map[0] + 0);
    _bn1_Constant_output_0_3D_array.data_start = AI_PTR(g_network_weights_map[0] + 0);
    _bn2_Constant_output_0_3D_array.format |= AI_FMT_FLAG_CONST;
    _bn2_Constant_output_0_3D_array.data = AI_PTR(g_network_weights_map[0] + 4);
    _bn2_Constant_output_0_3D_array.data_start = AI_PTR(g_network_weights_map[0] + 4);
    _fc1_Gemm_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _fc1_Gemm_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 8);
    _fc1_Gemm_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 8);
    _fc1_Gemm_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _fc1_Gemm_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 6408);
    _fc1_Gemm_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 6408);
    _bn1_ReduceMean_output_0_Mul_scale_array.format |= AI_FMT_FLAG_CONST;
    _bn1_ReduceMean_output_0_Mul_scale_array.data = AI_PTR(g_network_weights_map[0] + 8008);
    _bn1_ReduceMean_output_0_Mul_scale_array.data_start = AI_PTR(g_network_weights_map[0] + 8008);
    _bn1_ReduceMean_output_0_Mul_bias_array.format |= AI_FMT_FLAG_CONST;
    _bn1_ReduceMean_output_0_Mul_bias_array.data = AI_PTR(g_network_weights_map[0] + 8012);
    _bn1_ReduceMean_output_0_Mul_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 8012);
    _bn1_ReduceMean_1_output_0_Mul_bias_array.format |= AI_FMT_FLAG_CONST;
    _bn1_ReduceMean_1_output_0_Mul_bias_array.data = AI_PTR(g_network_weights_map[0] + 8016);
    _bn1_ReduceMean_1_output_0_Mul_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 8016);
    _bn1_Mul_output_0_scale_array.format |= AI_FMT_FLAG_CONST;
    _bn1_Mul_output_0_scale_array.data = AI_PTR(g_network_weights_map[0] + 8020);
    _bn1_Mul_output_0_scale_array.data_start = AI_PTR(g_network_weights_map[0] + 8020);
    _bn1_Mul_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _bn1_Mul_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 9620);
    _bn1_Mul_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 9620);
    _fc2_Gemm_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _fc2_Gemm_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 11220);
    _fc2_Gemm_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 11220);
    _fc2_Gemm_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _fc2_Gemm_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 491220);
    _fc2_Gemm_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 491220);
    _bn2_ReduceMean_output_0_Mul_scale_array.format |= AI_FMT_FLAG_CONST;
    _bn2_ReduceMean_output_0_Mul_scale_array.data = AI_PTR(g_network_weights_map[0] + 492420);
    _bn2_ReduceMean_output_0_Mul_scale_array.data_start = AI_PTR(g_network_weights_map[0] + 492420);
    _bn2_Mul_output_0_scale_array.format |= AI_FMT_FLAG_CONST;
    _bn2_Mul_output_0_scale_array.data = AI_PTR(g_network_weights_map[0] + 492424);
    _bn2_Mul_output_0_scale_array.data_start = AI_PTR(g_network_weights_map[0] + 492424);
    _bn2_Mul_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _bn2_Mul_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 493624);
    _bn2_Mul_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 493624);
    _mu_Gemm_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _mu_Gemm_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 494824);
    _mu_Gemm_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 494824);
    _mu_Gemm_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _mu_Gemm_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 497224);
    _mu_Gemm_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 497224);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_WEIGHTS);
  return false;
}


/**  PUBLIC APIs SECTION  *****************************************************/



AI_DEPRECATED
AI_API_ENTRY
ai_bool ai_network_get_info(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 8809856,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .params            = AI_STRUCT_INIT,
      .activations       = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xf394ddd2,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}



AI_API_ENTRY
ai_bool ai_network_get_report(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 8809856,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .map_signature     = AI_MAGIC_SIGNATURE,
      .map_weights       = AI_STRUCT_INIT,
      .map_activations   = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xf394ddd2,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}


AI_API_ENTRY
ai_error ai_network_get_error(ai_handle network)
{
  return ai_platform_network_get_error(network);
}


AI_API_ENTRY
ai_error ai_network_create(
  ai_handle* network, const ai_buffer* network_config)
{
  return ai_platform_network_create(
    network, network_config, 
    AI_CONTEXT_OBJ(&AI_NET_OBJ_INSTANCE),
    AI_TOOLS_API_VERSION_MAJOR, AI_TOOLS_API_VERSION_MINOR, AI_TOOLS_API_VERSION_MICRO);
}


AI_API_ENTRY
ai_error ai_network_create_and_init(
  ai_handle* network, const ai_handle activations[], const ai_handle weights[])
{
  ai_error err;
  ai_network_params params;

  err = ai_network_create(network, AI_NETWORK_DATA_CONFIG);
  if (err.type != AI_ERROR_NONE) {
    return err;
  }
  
  if (ai_network_data_params_get(&params) != true) {
    err = ai_network_get_error(*network);
    return err;
  }
#if defined(AI_NETWORK_DATA_ACTIVATIONS_COUNT)
  /* set the addresses of the activations buffers */
  for (ai_u16 idx=0; activations && idx<params.map_activations.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);
  }
#endif
#if defined(AI_NETWORK_DATA_WEIGHTS_COUNT)
  /* set the addresses of the weight buffers */
  for (ai_u16 idx=0; weights && idx<params.map_weights.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);
  }
#endif
  if (ai_network_init(*network, &params) != true) {
    err = ai_network_get_error(*network);
  }
  return err;
}


AI_API_ENTRY
ai_buffer* ai_network_inputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_inputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_buffer* ai_network_outputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_outputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_handle ai_network_destroy(ai_handle network)
{
  return ai_platform_network_destroy(network);
}


AI_API_ENTRY
ai_bool ai_network_init(
  ai_handle network, const ai_network_params* params)
{
  ai_network* net_ctx = AI_NETWORK_OBJ(ai_platform_network_init(network, params));
  ai_bool ok = true;

  if (!net_ctx) return false;
  ok &= network_configure_weights(net_ctx, params);
  ok &= network_configure_activations(net_ctx, params);

  ok &= ai_platform_network_post_init(network);

  return ok;
}


AI_API_ENTRY
ai_i32 ai_network_run(
  ai_handle network, const ai_buffer* input, ai_buffer* output)
{
  return ai_platform_network_process(network, input, output);
}


AI_API_ENTRY
ai_i32 ai_network_forward(ai_handle network, const ai_buffer* input)
{
  return ai_platform_network_process(network, input, NULL);
}



#undef AI_NETWORK_MODEL_SIGNATURE
#undef AI_NET_OBJ_INSTANCE
#undef AI_TOOLS_DATE_TIME
#undef AI_TOOLS_COMPILE_TIME

