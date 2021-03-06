//===-- EpiphanyMCInstLower.cpp - Convert Epiphany MachineInstr to an MCInst -==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Epiphany MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "EpiphanyMCInstLower.h"

#include "EpiphanyAsmPrinter.h"
#include "EpiphanyInstrInfo.h"
#include "MCTargetDesc/EpiphanyBaseInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

EpiphanyMCInstLower::EpiphanyMCInstLower(EpiphanyAsmPrinter &asmprinter)
  : AsmPrinter(asmprinter) {}

  void EpiphanyMCInstLower::Initialize(MCContext* C) {
    Ctx = C;
  }

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
    const MCOperand& Opnd1,
    const MCOperand& Opnd2 = MCOperand()) {
  Inst.setOpcode(Opc);
  Inst.addOperand(Opnd0);
  Inst.addOperand(Opnd1);
  if (Opnd2.isValid())
    Inst.addOperand(Opnd2);
}

// @LowerOperand
MCOperand EpiphanyMCInstLower::LowerOperand(const MachineOperand &MO,
    unsigned Offset) const {
  const MCSymbol *Symbol;
  MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_None;
  EpiphanyMCExpr::EpiphanyExprKind TargetKind = EpiphanyMCExpr::CEK_None;
  switch (MO.getTargetFlags()) {
    default:
      llvm_unreachable("Invalid target flags");
    case EpiphanyII::MO_NO_FLAG:
      break;
    case EpiphanyII::MO_HIGH:
      TargetKind = EpiphanyMCExpr::CEK_HIGH;
      break;
    case EpiphanyII::MO_LOW:
      TargetKind = EpiphanyMCExpr::CEK_LOW;
      break;
    case EpiphanyII::MO_GPREL:
      TargetKind = EpiphanyMCExpr::CEK_GPREL;
      break;
  }

  switch (MO.getType()) {
    default: 
      llvm_unreachable("unknown operand type in EpiphanyMCInstLower::LowerOperand");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands
      if (MO.isImplicit()) return MCOperand();
      return MCOperand::createReg(MO.getReg());
    case MachineOperand::MO_Immediate:
      return MCOperand::createImm(MO.getImm() + Offset);
    case MachineOperand::MO_RegisterMask:
      return MCOperand();
    case MachineOperand::MO_MachineBasicBlock:
      Symbol = MO.getMBB()->getSymbol();
      break;
    case MachineOperand::MO_GlobalAddress:
      Symbol = AsmPrinter.getSymbol(MO.getGlobal());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_BlockAddress:
      Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_ExternalSymbol:
      Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_MCSymbol:
      Symbol = MO.getMCSymbol();
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_JumpTableIndex:
      Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      Symbol = AsmPrinter.GetCPISymbol(MO.getIndex());
      Offset += MO.getOffset();
      break;
 }
 const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Kind, *Ctx);
  if (Offset) {
    // Assume offset is never negative.
    assert(Offset > 0);
    Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, *Ctx), *Ctx);
  }

  if (TargetKind != EpiphanyMCExpr::CEK_None) {
    Expr = EpiphanyMCExpr::create(TargetKind, Expr, *Ctx);
  }

  return MCOperand::createExpr(Expr);

}

void EpiphanyMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
