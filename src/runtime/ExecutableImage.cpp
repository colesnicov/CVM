/*============================================================================
*
*  Virtual Machine Executable Image class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#include <cstring>
#include <iostream>
#include <iomanip>
#include "runtime/VirtualMachine.h"

using namespace std;
using namespace vm;


ExecutableImage::ExecutableImage() {
	clear();
}

ExecutableImage::~ExecutableImage() {
	clear();
}


//-----------------------------------------------------------------------------
// Clears executable image
//-----------------------------------------------------------------------------
void ExecutableImage::clear() {
	image.clear();
	emitAddress = 0;
}

//-----------------------------------------------------------------------------
// Checks available space and resize image if required
//-----------------------------------------------------------------------------
void ExecutableImage::prepareSpace(WORD wordsCount) {
	WORD required = emitAddress + wordsCount;
	if (image.size() < required) image.resize(required);
}

void ExecutableImage::prepareSpace(WORD address, WORD wordsCount) {
	WORD required = address + wordsCount;
	if (image.size() < required) image.resize(required);
}


//-----------------------------------------------------------------------------
// Sets EmitAddress value to specified address
//-----------------------------------------------------------------------------
WORD ExecutableImage::setEmitAddress(WORD address) {
	return emitAddress = address;
}


//-----------------------------------------------------------------------------
// Returns EmitAddress value
//-----------------------------------------------------------------------------
WORD ExecutableImage::getEmitAddress() {
	return emitAddress;
}


//-----------------------------------------------------------------------------
// Writes opcode to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode) {
	prepareSpace(1);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	return startAddress;
}

//-----------------------------------------------------------------------------
// Writes opcode and its operand to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode, WORD operand) {
	prepareSpace(2);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	image[emitAddress++] = operand;
	return startAddress;
}

//-----------------------------------------------------------------------------
// Writes opcode and its operands to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode, WORD operand1, WORD operand2) {
	prepareSpace(3);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	image[emitAddress++] = operand1;
	image[emitAddress++] = operand2;
	return startAddress;
}


//-----------------------------------------------------------------------------
// Writes specified image to this executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(ExecutableImage& img) {
	WORD startAddress = emitAddress;
	WORD wordsCount = img.getSize();
	prepareSpace(wordsCount);
	memcpy(image.data() + emitAddress, img.getImage(), wordsCount * sizeof(WORD));
	emitAddress += wordsCount;
	return startAddress;
}


//-----------------------------------------------------------------------------
// Write WORD to specified memory address
//-----------------------------------------------------------------------------
void ExecutableImage::writeWord(WORD address, WORD value) {
	WORD temp = emitAddress;
	prepareSpace(address, 1);
	image[address] = value;
	emitAddress = temp;
}


//-----------------------------------------------------------------------------
// Writes data to executable image at current EmitAddress
//-----------------------------------------------------------------------------
void ExecutableImage::writeData(WORD address, void* data, WORD bytesCount) {
	WORD reminder = bytesCount % sizeof(WORD);
	WORD wordsCount = bytesCount / sizeof(WORD);
	if (reminder != 0) wordsCount++;
	prepareSpace(address, wordsCount);
	memcpy(image.data() + address, data, bytesCount);
}

//-----------------------------------------------------------------------------
// Reads WORD from executable image at specified EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::readWord(WORD address) {
	return image[address];
}


//-----------------------------------------------------------------------------
// Returns pointer to executable image
//-----------------------------------------------------------------------------
WORD* ExecutableImage::getImage() {
	return image.data();
}


//-----------------------------------------------------------------------------
// Returns memory size in words
//-----------------------------------------------------------------------------
WORD ExecutableImage::getSize() {
	return (WORD) image.size();
}


//-----------------------------------------------------------------------------
// Disassembles executable image to console
//-----------------------------------------------------------------------------
void ExecutableImage::disassemble() {
	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual machine executable image disassembly" << endl;
	cout << "-----------------------------------------------------" << endl;
	if (image.size() == 0) return;
	WORD opcode;
	WORD previousOp = -1;
	WORD ip = 0;
	do {
		opcode = image[ip];
		if (opcode != OP_HALT) ip += printMnemomic(ip);
		else {
			if (previousOp != OP_HALT) printMnemomic(ip);
			ip++;
		}
		previousOp = opcode;
	} while (ip < image.size());
}


//-----------------------------------------------------------------------------
// Prints instruction mnemonic
//-----------------------------------------------------------------------------
WORD ExecutableImage::printMnemomic(WORD address) {
	WORD ip = address;
	WORD opcode = image[ip++];
	cout << "[" << setw(6) << address << "]    ";
	switch (opcode) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
		case OP_CONST:	cout << "iconst  " << image[ip++]; break;
		case OP_PUSH:   cout << "ipush   [" << image[ip++] << "]"; break;
		case OP_POP:    cout << "ipop    [" << image[ip++] << "]"; break;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
		case OP_ADD:    cout << "iadd    "; break;
		case OP_SUB:    cout << "isub    "; break;
		case OP_MUL:    cout << "imul    "; break;
		case OP_DIV:    cout << "idiv    "; break;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
		case OP_AND:    cout << "iand    "; break;
		case OP_OR:     cout << "ior     "; break;
		case OP_XOR:    cout << "ixor    "; break;
		case OP_NOT:    cout << "inot    "; break;
		case OP_SHL:    cout << "ishl    "; break;
		case OP_SHR:    cout << "ishr    "; break;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS
		//------------------------------------------------------------------------
		case OP_JMP:    cout << "jmp     [" << std::showpos << image[ip++] << std::noshowpos << "]"; break;
		case OP_IFZERO: cout << "ifzero  [" << std::showpos << image[ip++] << std::noshowpos << "]"; break;
		case OP_EQUAL:  cout << "equal    "; break;
		case OP_NEQUAL: cout << "nequal   "; break;
		case OP_GREATER:cout << "greater  "; break;
		case OP_GREQUAL:cout << "grequal  "; break;
		case OP_LESS:   cout << "less     "; break;
		case OP_LSEQUAL:cout << "lsequal  "; break;
		case OP_LAND:   cout << "land  "; break;
		case OP_LOR:    cout << "lor   "; break;
		case OP_LNOT:   cout << "lnot  "; break;

		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
		case OP_CALL:   cout << "call    [" << image[ip++] << "], ";
			if(image.size() > ip){
				cout << image[ip++];
			}
		break;
		case OP_RET:    cout << "ret     "; break;
		case OP_SYSCALL:cout << "syscall 0x" << setbase(16) << image[ip++] << setbase(10); break;
		case OP_HALT: 	cout << "---- halt ----"; break;
		//------------------------------------------------------------------------
		// LOCAL VARIABLES AND ARGUMENTS ACCESS OPERATIONS
		//------------------------------------------------------------------------
		case OP_LOAD:	cout << "iload   #" << image[ip++]; break;
		case OP_STORE:	cout << "istore  #" << image[ip++]; break;
		case OP_ARG:	cout << "iarg    #" << image[ip++]; break;
	default:
		cout << "0x" << setbase(16) << opcode << setbase(10);
	}
	cout << endl;
	return ip - address;
}

