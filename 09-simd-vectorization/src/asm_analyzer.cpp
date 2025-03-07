#include "../include/asm_analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

// Map of SIMD instruction prefixes
const std::map<std::string, std::string> simdInstructionPrefixes = {
    // SSE instructions
    {"movaps", "SSE"},
    {"movups", "SSE"},
    {"movss", "SSE"},
    {"addps", "SSE"},
    {"subps", "SSE"},
    {"mulps", "SSE"},
    {"divps", "SSE"},
    {"sqrtps", "SSE"},
    {"maxps", "SSE"},
    {"minps", "SSE"},
    {"andps", "SSE"},
    {"orps", "SSE"},
    {"xorps", "SSE"},
    
    // SSE2 instructions
    {"movapd", "SSE2"},
    {"movupd", "SSE2"},
    {"movsd", "SSE2"},
    {"addpd", "SSE2"},
    {"subpd", "SSE2"},
    {"mulpd", "SSE2"},
    {"divpd", "SSE2"},
    {"sqrtpd", "SSE2"},
    {"maxpd", "SSE2"},
    {"minpd", "SSE2"},
    {"andpd", "SSE2"},
    {"orpd", "SSE2"},
    {"xorpd", "SSE2"},
    
    // AVX instructions
    {"vmovaps", "AVX"},
    {"vmovapd", "AVX"},
    {"vmovups", "AVX"},
    {"vmovupd", "AVX"},
    {"vaddps", "AVX"},
    {"vaddpd", "AVX"},
    {"vsubps", "AVX"},
    {"vsubpd", "AVX"},
    {"vmulps", "AVX"},
    {"vmulpd", "AVX"},
    {"vdivps", "AVX"},
    {"vdivpd", "AVX"},
    {"vsqrtps", "AVX"},
    {"vsqrtpd", "AVX"},
    {"vfmadd", "AVX"},
    {"vfmsub", "AVX"},
    
    // AVX2 instructions
    {"vgather", "AVX2"},
    {"vpgather", "AVX2"},
    {"vpermd", "AVX2"},
    {"vperm2i128", "AVX2"},
    {"vextracti128", "AVX2"},
    {"vinserti128", "AVX2"},
    
    // AVX-512 instructions
    {"zmm", "AVX-512"},
    {"vbroadcast", "AVX-512"},
    {"vcompress", "AVX-512"},
    {"vexpand", "AVX-512"},
    {"vextract", "AVX-512"},
    {"vinsert", "AVX-512"},
    {"vmask", "AVX-512"},
    {"vpermil", "AVX-512"},
};

// Parse an assembly file into structured data
std::vector<AsmInstruction> parseAsmFile(const std::string& filename) {
    std::vector<AsmInstruction> instructions;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return instructions;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        AsmInstruction instr;
        instr.isSIMD = false;
        
        // Simple parsing - this might need adjustment based on the actual assembly format
        std::istringstream iss(line);
        std::string token;
        std::string fullLine = line;
        
        // Try to extract the address (usually at the beginning, e.g., "00000000000001a0:")
        if (line.find(':') != std::string::npos) {
            instr.address = line.substr(0, line.find(':') + 1);
            line = line.substr(line.find(':') + 1);
        } else {
            instr.address = "";
        }
        
        // Skip leading whitespace
        line = std::regex_replace(line, std::regex("^\\s+"), "");
        
        // Extract the opcode (the first token after the address)
        std::istringstream lineStream(line);
        if (lineStream >> token) {
            instr.opcode = token;
            
            // Identify if this is a SIMD instruction
            for (const auto& prefix : simdInstructionPrefixes) {
                if (instr.opcode.find(prefix.first) != std::string::npos) {
                    instr.isSIMD = true;
                    break;
                }
            }
            
            // Extract operands (everything between opcode and comment)
            std::string operands;
            std::getline(lineStream, operands);
            
            // Split into operands and comment
            size_t commentPos = operands.find('#');
            if (commentPos != std::string::npos) {
                instr.operands = operands.substr(0, commentPos);
                instr.comment = operands.substr(commentPos);
            } else {
                instr.operands = operands;
                instr.comment = "";
            }
            
            // Trim whitespace
            instr.operands = std::regex_replace(instr.operands, std::regex("^\\s+|\\s+$"), "");
            instr.comment = std::regex_replace(instr.comment, std::regex("^\\s+|\\s+$"), "");
        }
        
        instructions.push_back(instr);
    }
    
    file.close();
    return instructions;
}

// Identify SIMD instructions in the assembly
void identifySIMDInstructions(std::vector<AsmInstruction>& instructions) {
    for (auto& instr : instructions) {
        instr.isSIMD = false;
        
        // Check if the instruction uses SIMD
        for (const auto& prefix : simdInstructionPrefixes) {
            if (instr.opcode.find(prefix.first) != std::string::npos) {
                instr.isSIMD = true;
                break;
            }
        }
        
        // Check for SIMD registers (xmm, ymm, zmm)
        if (!instr.isSIMD) {
            if (instr.operands.find("xmm") != std::string::npos ||
                instr.operands.find("ymm") != std::string::npos ||
                instr.operands.find("zmm") != std::string::npos) {
                instr.isSIMD = true;
            }
        }
    }
}

// Display assembly statistics
void displayAsmStatistics(const std::vector<AsmInstruction>& instructions) {
    // Count instructions
    int totalInstructions = instructions.size();
    int simdInstructions = 0;
    
    // Count by SIMD type
    std::map<std::string, int> simdTypeCount;
    for (const auto& prefix : simdInstructionPrefixes) {
        simdTypeCount[prefix.second] = 0;
    }
    
    // Extract and count SIMD instruction types
    for (const auto& instr : instructions) {
        if (instr.isSIMD) {
            simdInstructions++;
            
            for (const auto& prefix : simdInstructionPrefixes) {
                if (instr.opcode.find(prefix.first) != std::string::npos) {
                    simdTypeCount[prefix.second]++;
                    break;
                }
            }
        }
    }
    
    // Display statistics
    std::cout << "Total instructions: " << totalInstructions << std::endl;
    std::cout << "SIMD instructions: " << simdInstructions 
              << " (" << std::fixed << std::setprecision(1) 
              << (totalInstructions > 0 ? (100.0 * simdInstructions / totalInstructions) : 0.0) 
              << "%)" << std::endl;
    
    std::cout << "\nSIMD instruction types:" << std::endl;
    for (const auto& type : simdTypeCount) {
        if (type.second > 0) {
            std::cout << "- " << type.first << ": " << type.second 
                      << " (" << std::fixed << std::setprecision(1) 
                      << (simdInstructions > 0 ? (100.0 * type.second / simdInstructions) : 0.0) 
                      << "%)" << std::endl;
        }
    }
}

// Compare vectorized vs non-vectorized assembly
void compareVectorizedAssembly(const std::string& vectorizedFile, const std::string& scalarFile) {
    auto vecInstructions = parseAsmFile(vectorizedFile);
    auto scalarInstructions = parseAsmFile(scalarFile);
    
    identifySIMDInstructions(vecInstructions);
    identifySIMDInstructions(scalarInstructions);
    
    std::cout << "\nComparison of vectorized vs. non-vectorized assembly:" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    
    std::cout << "Vectorized assembly (" << vectorizedFile << "):" << std::endl;
    displayAsmStatistics(vecInstructions);
    
    std::cout << "\nScalar assembly (" << scalarFile << "):" << std::endl;
    displayAsmStatistics(scalarInstructions);
    
    // Instruction count comparison
    std::cout << "\nInstruction count comparison:" << std::endl;
    std::cout << "- Vectorized: " << vecInstructions.size() << " instructions" << std::endl;
    std::cout << "- Scalar: " << scalarInstructions.size() << " instructions" << std::endl;
    
    // If one has more or fewer instructions
    if (vecInstructions.size() != scalarInstructions.size()) {
        int diff = static_cast<int>(vecInstructions.size()) - static_cast<int>(scalarInstructions.size());
        std::cout << "- Difference: " << std::abs(diff) << " "
                  << (diff < 0 ? "fewer" : "more") << " instructions in vectorized code" << std::endl;
    }
    
    // SIMD instruction usage
    int vecSIMDCount = 0;
    int scalarSIMDCount = 0;
    
    for (const auto& instr : vecInstructions) {
        if (instr.isSIMD) vecSIMDCount++;
    }
    
    for (const auto& instr : scalarInstructions) {
        if (instr.isSIMD) scalarSIMDCount++;
    }
    
    std::cout << "\nSIMD instruction usage:" << std::endl;
    std::cout << "- Vectorized: " << vecSIMDCount << " SIMD instructions" << std::endl;
    std::cout << "- Scalar: " << scalarSIMDCount << " SIMD instructions" << std::endl;
    
    // Significant difference
    if (vecSIMDCount > scalarSIMDCount) {
        std::cout << "\nThe vectorized code is using " << (vecSIMDCount - scalarSIMDCount) 
                  << " more SIMD instructions, which indicates successful vectorization." << std::endl;
    } else if (vecSIMDCount < scalarSIMDCount) {
        std::cout << "\nWarning: The vectorized code has fewer SIMD instructions than the scalar version." << std::endl;
        std::cout << "This might indicate that vectorization was not successful." << std::endl;
    } else if (vecSIMDCount == 0) {
        std::cout << "\nWarning: No SIMD instructions detected in either file." << std::endl;
        std::cout << "This might indicate that vectorization was not successful." << std::endl;
    } else {
        std::cout << "\nBoth files have the same number of SIMD instructions. Further analysis is needed." << std::endl;
    }
}

// Generate a human-readable annotated assembly file
void generateAnnotatedAsm(const std::string& inputFile, const std::string& outputFile) {
    auto instructions = parseAsmFile(inputFile);
    identifySIMDInstructions(instructions);
    
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFile << std::endl;
        return;
    }
    
    // Write header
    outFile << ";;; Annotated Assembly File - Generated by OpenMP SIMD Vectorization Analyzer ;;;" << std::endl;
    outFile << ";;; Source: " << inputFile << " ;;;" << std::endl;
    outFile << ";;; Legend: [SIMD] prefix indicates a SIMD instruction ;;;" << std::endl;
    outFile << std::endl;
    
    // Process and annotate each instruction
    for (const auto& instr : instructions) {
        // Prefix for SIMD instructions
        std::string prefix = instr.isSIMD ? "[SIMD] " : "       ";
        
        // Determine SIMD type if applicable
        std::string simdType = "";
        if (instr.isSIMD) {
            for (const auto& prefixPair : simdInstructionPrefixes) {
                if (instr.opcode.find(prefixPair.first) != std::string::npos) {
                    simdType = " [" + prefixPair.second + "]";
                    break;
                }
            }
        }
        
        // Format the instruction
        outFile << prefix << std::left << std::setw(12) << instr.address 
                << std::setw(15) << instr.opcode << " " 
                << std::setw(30) << instr.operands;
                
        // Add SIMD type and comment
        if (!instr.comment.empty()) {
            outFile << " " << instr.comment;
        }
        
        if (!simdType.empty()) {
            outFile << " " << simdType;
        }
        
        outFile << std::endl;
    }
    
    // Add statistics at the end
    outFile << std::endl << ";;; Assembly Statistics ;;;" << std::endl;
    
    int totalInstructions = instructions.size();
    int simdInstructions = 0;
    
    std::map<std::string, int> simdTypeCount;
    for (const auto& prefix : simdInstructionPrefixes) {
        simdTypeCount[prefix.second] = 0;
    }
    
    for (const auto& instr : instructions) {
        if (instr.isSIMD) {
            simdInstructions++;
            
            for (const auto& prefix : simdInstructionPrefixes) {
                if (instr.opcode.find(prefix.first) != std::string::npos) {
                    simdTypeCount[prefix.second]++;
                    break;
                }
            }
        }
    }
    
    outFile << "Total instructions: " << totalInstructions << std::endl;
    outFile << "SIMD instructions: " << simdInstructions 
            << " (" << std::fixed << std::setprecision(1) 
            << (totalInstructions > 0 ? (100.0 * simdInstructions / totalInstructions) : 0.0) 
            << "%)" << std::endl;
    
    outFile << "\nSIMD instruction types:" << std::endl;
    for (const auto& type : simdTypeCount) {
        if (type.second > 0) {
            outFile << "- " << type.first << ": " << type.second 
                    << " (" << std::fixed << std::setprecision(1) 
                    << (simdInstructions > 0 ? (100.0 * type.second / simdInstructions) : 0.0) 
                    << "%)" << std::endl;
        }
    }
    
    outFile.close();
    std::cout << "Annotated assembly written to " << outputFile << std::endl;
}

// Run the ASM analysis 
void runASMAnalysis() {
    std::cout << "\n=== Assembly Output Analysis ===" << std::endl;
    
    // Check if asm_output directory exists
    if (!fs::exists("asm_output")) {
        std::cout << "Assembly output directory not found. You need to generate assembly output first." << std::endl;
        std::cout << "Run 'generate_asm_report.bat' or build with the 'generate_asm' target to create assembly files." << std::endl;
        return;
    }
    
    // Find available assembly files
    std::vector<std::string> asmFiles;
    for (const auto& entry : fs::directory_iterator("asm_output")) {
        if (entry.path().extension() == ".asm") {
            asmFiles.push_back(entry.path().string());
        }
    }
    
    if (asmFiles.empty()) {
        std::cout << "No assembly files found in the asm_output directory." << std::endl;
        return;
    }
    
    // Display available files
    std::cout << "Found " << asmFiles.size() << " assembly files:" << std::endl;
    for (size_t i = 0; i < asmFiles.size(); i++) {
        std::cout << i + 1 << ". " << fs::path(asmFiles[i]).filename().string() << std::endl;
    }
    
    // Analyze each file
    std::cout << "\nAnalyzing assembly files..." << std::endl;
    
    for (const auto& file : asmFiles) {
        std::cout << "\nAnalyzing " << fs::path(file).filename().string() << ":" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        auto instructions = parseAsmFile(file);
        identifySIMDInstructions(instructions);
        displayAsmStatistics(instructions);
        
        // Generate annotated version
        std::string outputFile = "asm_output/annotated_" + fs::path(file).filename().string();
        generateAnnotatedAsm(file, outputFile);
    }
    
    // If we have at least 2 files, compare them
    if (asmFiles.size() >= 2) {
        std::cout << "\nWould you like to compare two assembly files? (y/n): ";
        char response;
        std::cin >> response;
        
        if (response == 'y' || response == 'Y') {
            int file1, file2;
            
            std::cout << "Enter the number of the first file: ";
            std::cin >> file1;
            
            std::cout << "Enter the number of the second file: ";
            std::cin >> file2;
            
            if (file1 >= 1 && file1 <= asmFiles.size() && 
                file2 >= 1 && file2 <= asmFiles.size() && 
                file1 != file2) {
                compareVectorizedAssembly(asmFiles[file1 - 1], asmFiles[file2 - 1]);
            } else {
                std::cout << "Invalid file selection." << std::endl;
            }
        }
    }
    
    std::cout << "\nAssembly analysis complete. Annotated files have been created in the asm_output directory." << std::endl;
}