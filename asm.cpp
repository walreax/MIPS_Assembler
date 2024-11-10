/*
AUTHOR: AKSHAT JHA
ROLL NUMBER 2301AI53
*/


#include <bits/stdc++.h>

using namespace std;

struct Instruction {
    string label;
    string mnemonic;
    string operand;
    int32_t programCounter;
    int32_t value;
    
    Instruction() : label(""), mnemonic(""), operand(""), programCounter(0), value(0) {}
};//STRUCT FOR INSTRUCTION

class Assembler {
private:
    int currentLine;
    int programCounter;
    int position;
    
    vector<pair<string, int>> errors;//ERRORS TABLE
    vector<pair<string, int>> warnings;//WARNING TABLE
    vector<Instruction> instructions;
    map<string, int32_t> symbolTable;
    map<string, uint8_t> mnemonicTable;
    bool logFileCreated;//CREATION OF .LOG FILE
    string currentLogFile;

    //THIS PART WRITES ERRORS AND WARNINGS TO THE .LOG FILE
    void logError(const string& message, int line) {
        errors.push_back(make_pair(message, line));
        writeToLog("Error", message, line);
    }

    void logWarning(const string& message, int line) {
        warnings.push_back(make_pair(message, line));
        writeToLog("Warning", message, line);
    }

    void writeToLog(const string& type, const string& message, int line) {
        if (!logFileCreated || currentLogFile.empty()) {
            return;
        }

        ofstream logFile(currentLogFile, ios::app);
        if (!logFile) {
            cerr << "Failed to write to log file: " << currentLogFile << endl;
            return;
        }

        logFile << type << " at line " << line << ": " << message << endl;
        logFile.close();
    }

    void initMnemonicTable() {
        mnemonicTable["data"] = 19;
        mnemonicTable["ldc"] = 0;
        mnemonicTable["adc"] = 1;
        mnemonicTable["ldl"] = 2;
        mnemonicTable["stl"] = 3;
        mnemonicTable["ldnl"] = 4;
        mnemonicTable["stnl"] = 5;
        mnemonicTable["add"] = 6;
        mnemonicTable["sub"] = 7;
        mnemonicTable["shl"] = 8;
        mnemonicTable["shr"] = 9;
        mnemonicTable["adj"] = 10;
        mnemonicTable["a2sp"] = 11;
        mnemonicTable["sp2a"] = 12;
        mnemonicTable["call"] = 13;
        mnemonicTable["return"] = 14;
        mnemonicTable["brz"] = 15;
        mnemonicTable["brlz"] = 16;
        mnemonicTable["br"] = 17;
        mnemonicTable["HALT"] = 18;
        mnemonicTable["SET"] = 20;
    }

    bool parseNumber(const string& str, int32_t& value) {
        try {
            if (str.length() > 2 && str.substr(0, 2) == "0x") {
                // Handle hexadecimal
                string hexPart = str.substr(2);
                char* endPtr;
                value = strtol(hexPart.c_str(), &endPtr, 16);
                return *endPtr == '\0';
            } else if (str.length() > 1 && str[0] == '0') {
                // Handle octal
                char* endPtr;
                value = strtol(str.c_str(), &endPtr, 8);
                return *endPtr == '\0';
            } else {
                // Handle decimal
                char* endPtr;
                value = strtol(str.c_str(), &endPtr, 10);
                return *endPtr == '\0';
            }
        } catch (...) {
            return false;
        }
    }

    bool isValidIdentifier(const string& str) {
        if (str.empty() || !isalpha(str[0])) {
            return false;
        }
        return all_of(str.begin() + 1, str.end(), 
                     [](char c) { return isalnum(c) || c == '_'; });
    }

    string parseToken(const string& line) {
        while (position < static_cast<int>(line.length()) && 
               (isspace(line[position]) || line[position] == ',')) {
            position++;
        }
        
        if (position >= static_cast<int>(line.length())) {
            return "";
        }
        
        string token;
        size_t colonPos = line.find(':', position);
        if (colonPos != string::npos) {
            token = line.substr(position, colonPos - position + 1);
            position = colonPos + 1;
            return token;
        }
        
        while (position < static_cast<int>(line.length()) && 
               !isspace(line[position]) && line[position] != ',') {
            token += line[position++];
        }
        return token;
    }

    bool requiresOperand(const string& mnemonic) {
        static const set<string> noOperandMnes = {
            "add", "sub", "shl", "shr", "a2sp", "sp2a", "return", "HALT"
        };
        return noOperandMnes.find(mnemonic) == noOperandMnes.end();
    }

    void processLine(const string& line) {
        position = 0;
        Instruction inst;
        inst.programCounter = programCounter;

        string trimmedLine = line;
        size_t commentPos = trimmedLine.find(';');
        if (commentPos != string::npos) {
            trimmedLine = trimmedLine.substr(0, commentPos);
        }
        if (trimmedLine.empty()) {
            return;
        }

        string token = parseToken(trimmedLine);
        if (token.empty()) return;

        if (token.back() == ':') {
            string labelName = token.substr(0, token.length() - 1);
            if (!isValidIdentifier(labelName)) {
                logError("Invalid label name: " + labelName, currentLine);
                return;
            }
            inst.label = token;
            if (symbolTable.find(labelName) != symbolTable.end()) {
                logError("Duplicate label: " + labelName, currentLine);
                return;
            }
            symbolTable[labelName] = programCounter;
            token = parseToken(trimmedLine);
        }

        if (token.empty()) {
            instructions.push_back(inst);
            return;
        }

        if (mnemonicTable.find(token) == mnemonicTable.end()) {
            logError("Invalid mnemonic: " + token, currentLine);
            return;
        }
        inst.mnemonic = token;

        if (requiresOperand(token)) {
            token = parseToken(trimmedLine);
            if (token.empty()) {
                logError("Missing operand for " + inst.mnemonic, currentLine);
                return;
            }
            inst.operand = token;
            
            if (!parseOperand(inst)) {
                return;
            }
        }

        instructions.push_back(inst);
        if (inst.mnemonic != "SET") {
            programCounter++;
        }
    }

    bool parseOperand(Instruction& inst) {
        int32_t value;
        if (parseNumber(inst.operand, value)) {
            if (value > 0x7FFFFF || value < -0x800000) {
                logError("Operand value out of range (must fit in 24 bits): " + inst.operand,
                    currentLine);
                return false;
            }
            if (value > 0x3FFFFF || value < -0x400000) {
                logWarning("Large operand value (approaching 24-bit limit): " + inst.operand,
                    currentLine);
            }
            inst.value = value;
            return true;
        }
        
        if (isValidIdentifier(inst.operand)) {
            inst.value = 0;
            return true;
        }
        
        logError("Invalid operand: " + inst.operand, currentLine);
        return false;
    }

    bool resolveSymbols() {
        bool success = true;
        set<string> usedLabels;

        for (Instruction& inst : instructions) {
            if (!inst.operand.empty() && isValidIdentifier(inst.operand)) {
                auto it = symbolTable.find(inst.operand);
                if (it == symbolTable.end()) {
                    logError("Undefined label: " + inst.operand, currentLine);
                    success = false;
                    continue;
                }
                
                usedLabels.insert(inst.operand);
                
                if (inst.mnemonic == "br" || inst.mnemonic == "brz" || 
                    inst.mnemonic == "brlz" || inst.mnemonic == "call") {
                    inst.value = it->second - (inst.programCounter + 1);
                } else {
                    inst.value = it->second;
                }
                
                if (inst.value > 0x7FFFFF || inst.value < -0x800000) {
                    logError("Resolved value out of range (must fit in 24 bits): " + 
                        to_string(inst.value), currentLine);
                    success = false;
                }
            }
        }

        // Check for unused labels
        for (const auto& symbol : symbolTable) {
            if (usedLabels.find(symbol.first) == usedLabels.end()) {
                logWarning("Unused label: " + symbol.first, 0);
            }
        }

        return success;
    }

    uint32_t encodeInstruction(uint8_t opcode, int32_t operand) {
        uint32_t maskedOperand = operand & 0x00FFFFFF;
        return (maskedOperand << 8) | (opcode & 0xFF);
    }

public:
    Assembler() : currentLine(1), programCounter(0), position(0), logFileCreated(false) {
        initMnemonicTable();
    }

    bool assemble(const string& inputFile) {
        ifstream inFile(inputFile);
        if (!inFile) {
            logError("Could not open input file: " + inputFile, 0);
            return false;
        }

        string baseFileName = inputFile.substr(0, inputFile.find_last_of('.'));
        currentLogFile = baseFileName + ".log";
        
        // Create new log file
        ofstream logFile(currentLogFile);
        if (!logFile) {
            cerr << "Failed to create log file: " << currentLogFile << endl;
            return false;
        }
        logFile << "Assembly started for file: " << inputFile << endl;
        logFile << "Timestamp: " << time(nullptr) << endl << endl;
        logFile.close();
        
        logFileCreated = true;

        string line;
        while (getline(inFile, line)) {
            processLine(line);
            currentLine++;
        }

        return resolveSymbols();
    }

    void generateOutput(const string& baseFileName) {
        // Generate listing file
        ofstream listFile(baseFileName + ".lst");
        if (!listFile) {
            logError("Failed to create listing file: " + baseFileName + ".lst", 0);
            return;
        }

        for (const Instruction& inst : instructions) {
            listFile << hex << setfill('0') << setw(8) << inst.programCounter << "  ";
            
            if (!inst.mnemonic.empty() && inst.mnemonic != "SET") {
                uint32_t encoded = encodeInstruction(
                    mnemonicTable[inst.mnemonic],
                    inst.value
                );
                listFile << setw(8) << encoded << "  ";
            } else {
                listFile << "          ";
            }

            if (!inst.label.empty()) listFile << inst.label << " ";
            if (!inst.mnemonic.empty()) listFile << inst.mnemonic << " ";
            if (!inst.operand.empty()) listFile << inst.operand;
            listFile << endl;
        }

        // Generate object file if no errors
        if (!hasErrors()) {
            ofstream objFile(baseFileName + ".o", ios::binary);
            if (!objFile) {
                logError("Failed to create object file: " + baseFileName + ".o", 0);
                return;
            }

            for (const Instruction& inst : instructions) {
                if (!inst.mnemonic.empty() && inst.mnemonic != "SET") {
                    uint32_t encoded = encodeInstruction(
                        mnemonicTable[inst.mnemonic],
                        inst.value
                    );
                    objFile.write(reinterpret_cast<const char*>(&encoded), sizeof(uint32_t));
                }
            }
        }

        // Write final summary to log file
        if (logFileCreated) {
            ofstream logFile(currentLogFile, ios::app);
            if (logFile) {
                logFile << "\nAssembly completed with:" << endl;
                logFile << "- " << errors.size() << " error(s)" << endl;
                logFile << "- " << warnings.size() << " warning(s)" << endl;
            }
        }
    }

    bool hasErrors() const {
        return !errors.empty();
    }

    size_t getErrorCount() const {
        return errors.size();
    }

    size_t getWarningCount() const {
        return warnings.size();
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input.asm>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string baseFileName = inputFile.substr(0, inputFile.find_last_of('.'));

    Assembler assembler;
    bool success = assembler.assemble(inputFile);
    assembler.generateOutput(baseFileName);
    
    if (!success) {
        cout << "Assembly failed with " << assembler.getErrorCount() << " error(s) and "
             << assembler.getWarningCount() << " warning(s)" << endl;
        cout << "Check " << baseFileName << ".log for details" << endl;
    }
    
    return success ? 0 : 1;
}