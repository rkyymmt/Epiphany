//===-- EpiphanyInstPrinter.h - Convert Epiphany MCInst to assembly syntax --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an Epiphany MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EPIPHANYINSTPRINTER_H
#define LLVM_EPIPHANYINSTPRINTER_H

#include "MCTargetDesc/EpiphanyMCTargetDesc.h"
#include "Utils/EpiphanyBaseInfo.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCSubtargetInfo.h"

namespace llvm {

	class MCOperand;

	class EpiphanyInstPrinter : public MCInstPrinter {
	public:
		EpiphanyInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
			const MCRegisterInfo &MRI, const MCSubtargetInfo &STI);

		// Autogenerated by tblgen
		void printInstruction(const MCInst *MI, raw_ostream &O);
		bool printAliasInstr(const MCInst *MI, raw_ostream &O);
		static const char *getRegisterName(unsigned RegNo);
		static const char *getInstructionName(unsigned Opcode);

		void printAddSubImmOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		void printBareImmOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		void printCondCodeOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		template<int MemScale>
		void printOffsetUImm11Operand(const MCInst *MI, unsigned OpNum, raw_ostream &o) { printOffsetUImm11Operand(MI, OpNum, o, MemScale);	}
		void printOffsetUImm11Operand(const MCInst *MI, unsigned OpNum, raw_ostream &o, int MemScale);

		template<unsigned field_width, unsigned scale>
		void printLabelOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		template<unsigned RegWidth>
		void printLogicalImmOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		template<int MemSize> void
		printSImm7ScaledOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		void printOffsetSImm9Operand(const MCInst *MI, unsigned OpNum, raw_ostream &O);

		void printFPImmOperand(const MCInst *MI, unsigned OpNum, raw_ostream &o);

		void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
		virtual void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot);

		bool isStackReg(unsigned RegNo) {
			return RegNo == Epiphany::SP;
		}


	};

}

#endif
