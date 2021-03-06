/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include "frontend/A64/translate/impl/impl.h"

namespace Dynarmic::A64 {

bool TranslatorVisitor::STP_LDP_gen(Imm<2> opc, bool not_postindex, bool wback, Imm<1> L, Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt) {
    const bool postindex = !not_postindex;

    const MemOp memop = L == 1 ? MemOp::LOAD : MemOp::STORE;
    if ((L == 0 && opc.Bit<0>() == 1) || opc == 0b11)
        return UnallocatedEncoding();
    const bool signed_ = opc.Bit<0>() != 0;
    const size_t scale = 2 + opc.Bit<1>();
    const size_t datasize = 8 << scale;
    const u64 offset = imm7.SignExtend<u64>() << scale;

    if (memop == MemOp::LOAD && wback && (Rt == Rn || Rt2 == Rn) && Rn != Reg::R31)
        return UnpredictableInstruction();
    if (memop == MemOp::STORE && wback && (Rt == Rn || Rt2 == Rn) && Rn != Reg::R31)
        return UnpredictableInstruction();
    if (memop == MemOp::LOAD && Rt == Rt2)
        return UnpredictableInstruction();

    IR::U64 address;
    const size_t dbytes = datasize / 8;

    if (Rn == Reg::SP)
        // TODO: Check SP Alignment
        address = SP(64);
    else
        address = X(64, Rn);

    if (!postindex)
        address = ir.Add(address, ir.Imm64(offset));

    switch (memop) {
    case MemOp::STORE: {
        IR::U32U64 data1 = X(datasize, Rt);
        IR::U32U64 data2 = X(datasize, Rt2);
        Mem(address, dbytes, AccType::NORMAL, data1);
        Mem(ir.Add(address, ir.Imm64(dbytes)), dbytes, AccType::NORMAL, data2);
        break;
    }
    case MemOp::LOAD: {
        IR::U32U64 data1 = Mem(address, dbytes, AccType::NORMAL);
        IR::U32U64 data2 = Mem(ir.Add(address, ir.Imm64(dbytes)), dbytes, AccType::NORMAL);
        if (signed_) {
            X(64, Rt, SignExtend(data1, 64));
            X(64, Rt2, SignExtend(data2, 64));
        } else {
            X(datasize, Rt, data1);
            X(datasize, Rt2, data2);
        }
        break;
    }
    case MemOp::PREFETCH:
        UNREACHABLE();
    }

    if (wback) {
        if (postindex)
            address = ir.Add(address, ir.Imm64(offset));
        if (Rn == Reg::SP)
            SP(64, address);
        else
            X(64, Rn, address);
    }

    return true;
}

bool TranslatorVisitor::STP_LDP_fpsimd(Imm<2> opc, bool not_postindex, bool wback, Imm<1> L, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt) {
    const bool postindex = !not_postindex;

    const MemOp memop = L == 1 ? MemOp::LOAD : MemOp::STORE;
    if (opc == 0b11)
        return UnallocatedEncoding();
    const size_t scale = 2 + opc.ZeroExtend<size_t>();
    const size_t datasize = 8 << scale;
    const u64 offset = imm7.SignExtend<u64>() << scale;

    const size_t dbytes = datasize / 8;

    if (memop == MemOp::LOAD && Vt == Vt2)
        return UnpredictableInstruction();

    IR::U64 address;

    if (Rn == Reg::SP)
        // TODO: Check SP Alignment
        address = SP(64);
    else
        address = X(64, Rn);

    if (!postindex)
        address = ir.Add(address, ir.Imm64(offset));

    switch (memop) {
    case MemOp::STORE: {
        IR::UAnyU128 data1 = V(datasize, Vt);
        IR::UAnyU128 data2 = V(datasize, Vt2);
        if (datasize != 128) {
            data1 = ir.VectorGetElement(datasize, data1, 0);
            data2 = ir.VectorGetElement(datasize, data2, 0);
        }
        Mem(address, dbytes, AccType::VEC, data1);
        Mem(ir.Add(address, ir.Imm64(dbytes)), dbytes, AccType::VEC, data2);
        break;
    }
    case MemOp::LOAD: {
        IR::UAnyU128 data1 = Mem(address, dbytes, AccType::VEC);
        IR::UAnyU128 data2 = Mem(ir.Add(address, ir.Imm64(dbytes)), dbytes, AccType::VEC);
        if (datasize != 128) {
            data1 = ir.ZeroExtendToQuad(data1);
            data2 = ir.ZeroExtendToQuad(data2);
        }
        V(datasize, Vt, data1);
        V(datasize, Vt2, data2);
        break;
    }
    case MemOp::PREFETCH:
        UNREACHABLE();
    }

    if (wback) {
        if (postindex)
            address = ir.Add(address, ir.Imm64(offset));
        if (Rn == Reg::SP)
            SP(64, address);
        else
            X(64, Rn, address);
    }

    return true;
}

} // namespace Dynarmic::A64
