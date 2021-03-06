#
# Copyright (c) 2015-2017, Renesas Electronics Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   - Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
#   - Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#   - Neither the name of Renesas nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v10.c
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v11.c
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v20.c
    BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v10.c
    BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v11.c
    BL2_SOURCES += plat/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
else ifdef RCAR_LSI_CUT_COMPAT
  ifeq (${RCAR_LSI},${RCAR_H3})
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v10.c
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v11.c
    BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v20.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3})
    BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v10.c
    BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v11.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    BL2_SOURCES += plat/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
  endif
else
  ifeq (${RCAR_LSI},${RCAR_H3})
    ifeq (${LSI_CUT},10)
      BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v10.c
    else ifeq (${LSI_CUT},11)
      BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v11.c
    else ifeq (${LSI_CUT},20)
      BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v20.c
    else
#     LSI_CUT 20 or later
      BL2_SOURCES += plat/renesas/rcar/qos/H3/qos_init_h3_v20.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_M3})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v10.c
    else ifeq (${LSI_CUT},11)
     BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v11.c
    else
#    LSI_CUT 11 or later
     BL2_SOURCES += plat/renesas/rcar/qos/M3/qos_init_m3_v11.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += plat/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
    else
#    LSI_CUT 10 or later
     BL2_SOURCES += plat/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
    endif
  endif
endif

BL2_SOURCES += plat/renesas/rcar/qos/qos_init.c
