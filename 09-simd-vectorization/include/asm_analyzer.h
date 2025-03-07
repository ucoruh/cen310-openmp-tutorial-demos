#ifndef ASM_ANALYZER_H
#define ASM_ANALYZER_H

#include <string>
#include <vector>

// Structure to represent an assembly instruction
struct AsmInstruction {
    std::string address;
    std::string opcode;
    std::string operands;
    std::string comment;
    bool isSIMD;
};

// Main function to run ASM analysis
void runASMAnalysis();

// Parse an assembly file into structured data
std::vector<AsmInstruction> parseAsmFile(const std::string& filename);

// Identify SIMD instructions in the assembly
void identifySIMDInstructions(std::vector<AsmInstruction>& instructions);

// Compare vectorized vs non-vectorized assembly
void compareVectorizedAssembly(const std::string& vectorizedFile, const std::string& scalarFile);

// Generate a human-readable annotated assembly file
void generateAnnotatedAsm(const std::string& inputFile, const std::string& outputFile);

// Display assembly statistics
void displayAsmStatistics(const std::vector<AsmInstruction>& instructions);

#endif // ASM_ANALYZER_H