/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include "common/bit_util.h"
#include "frontend/A64/translate/impl/impl.h"

namespace Dynarmic::A64 {

bool TranslatorVisitor::ADD_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd) {
    if (size != 0b11) {
        return ReservedValue();
    }
    const size_t esize = 8 << size.ZeroExtend<size_t>();
    const size_t datasize = esize;

    const IR::U64 operand1 = V_scalar(datasize, Vn);
    const IR::U64 operand2 = V_scalar(datasize, Vm);
    const IR::U64 result = ir.Add(operand1, operand2);
    V_scalar(datasize, Vd, result);
    return true;
}

bool TranslatorVisitor::SUB_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd) {
    if (size != 0b11) {
        return ReservedValue();
    }
    const size_t esize = 8 << size.ZeroExtend<size_t>();
    const size_t datasize = esize;

    const IR::U64 operand1 = V_scalar(datasize, Vn);
    const IR::U64 operand2 = V_scalar(datasize, Vm);
    const IR::U64 result = ir.Sub(operand1, operand2);
    V_scalar(datasize, Vd, result);
    return true;
}

} // namespace Dynarmic::A64
