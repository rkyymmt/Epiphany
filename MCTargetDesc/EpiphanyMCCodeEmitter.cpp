//===-- EpiphanyMCCodeEmitter.cpp - Convert Epiphany Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the EpiphanyMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#include "EpiphanyMCCodeEmitter.h"

#include "MCTargetDesc/EpiphanyBaseInfo.h"
#include "MCTargetDesc/EpiphanyFixupKinds.h"
#include "MCTargetDesc/EpiphanyMCExpr.h"
#include "MCTargetDesc/EpiphanyMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "EpiphanyGenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

namespace llvm {
  MCCodeEmitter *createEpiphanyMCCodeEmitterEL(const MCInstrInfo &MCII,
      const MCRegisterInfo &MRI,
      MCContext &Ctx) {
    return new EpiphanyMCCodeEmitter(MCII, Ctx, true);
  }
} // End of namespace llvm

void EpiphanyMCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
  OS << (char)C;
}

void EpiphanyMCCodeEmitter::EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
  // Output the instruction encoding in little endian byte order.
  for (unsigned i = 0; i < Size; ++i) {
    unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
    EmitByte((Val >> Shift) & 0xff, OS);
  }
}

/// encodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes)
void EpiphanyMCCodeEmitter::
encodeInstruction(const MCInst &MI, raw_ostream &OS,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

  // Check for unimplemented opcodes.
  unsigned Opcode = MI.getOpcode();
  if (!Binary) {
    dbgs() << "Opcode: " << Opcode << "\n";
    dbgs() << "Binary: " << Binary << "\n";
    llvm_unreachable("unimplemented opcode in encodeInstruction()");
  }

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  // Pseudo instructions don't get encoded and shouldn't be here
  // in the first place!
  if ((TSFlags & EpiphanyII::FormMask) == EpiphanyII::Pseudo) {
    llvm_unreachable("Pseudo opcode found in encodeInstruction()");
  }

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  EmitInstruction(Binary, Size, OS);
}

//@CH8_1 {
/// getBranch16TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned EpiphanyMCCodeEmitter::
getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  return 0;
}

/// getBranch24TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned EpiphanyMCCodeEmitter::
getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand, such as JSUB. 
/// If the machine operand requires relocation,
/// record the relocation and return zero.
//@getJumpTargetOpValue {
unsigned EpiphanyMCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  return 0;
}
//@CH8_1 }

//@getExprOpValue {
unsigned EpiphanyMCCodeEmitter::
getExprOpValue(const MCExpr *Expr,SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  //@getExprOpValue body {
  MCExpr::ExprKind Kind = Expr->getKind();
  if (Kind == MCExpr::Constant) {
    return cast<MCConstantExpr>(Expr)->getValue();
  }

  if (Kind == MCExpr::Binary) {
    unsigned Res = getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
    Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
    return Res;
  }

  if (Kind == MCExpr::Target) {
    const EpiphanyMCExpr *EpiphanyExpr = cast<EpiphanyMCExpr>(Expr);

    Epiphany::Fixups FixupKind = Epiphany::Fixups(0);
    switch (EpiphanyExpr->getKind()) {
      default: llvm_unreachable("Unsupported fixup kind for target expression!");
    } // switch
    Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(FixupKind)));
    return 0;
  }

  // All of the information is in the fixup.
  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned EpiphanyMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  }
  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MO.getExpr(),Fixups, STI);
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
EpiphanyMCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups, STI);

  return (OffBits & 0xFFFF) | RegBits;
}

#include "EpiphanyGenMCCodeEmitter.inc"

