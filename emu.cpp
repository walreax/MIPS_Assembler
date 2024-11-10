/*
AUTHOR: AKSHAT JHA
ROLL NUMBER 2301AI53
*/
#include <bits/stdc++.h>

using namespace std;

// Constants
const int MEMORY_SIZE = 65536;  // 64K memory
const int MAX_STACK = 10000;    // Maximum stack size

// CPU class definition
class CPU {
private:
    int32_t A, B;       // Accumulator registers
    int32_t PC;         // Program counter
    int32_t SP;         // Stack pointer
    vector<int32_t> memory;
    bool halted;
    ofstream traceFile;  // File stream for trace output
    
    int lastPC; // previously tracked PC
    int instruction_count = 0; // count number of instructions executed
    const int LOOP_DETECTION_THRESHOLD = 1000; // max number of instructions before loop warning
    set<int> visited_pcs; // set to track visited program counters
    int sameLocationCount; // Count how many times we've been at same location
    
    // Instruction set
    const map<int, pair<string, string>> instructions = {
        {0, {"ldc", "Load constant"}},
        {1, {"adc", "Add constant"}},
        {2, {"ldl", "Load local"}},
        {3, {"stl", "Store local"}},
        {4, {"ldnl", "Load non-local"}},
        {5, {"stnl", "Store non-local"}},
        {6, {"add", "Add"}},
        {7, {"sub", "Subtract"}},
        {8, {"shl", "Shift left"}},
        {9, {"shr", "Shift right"}},
        {10, {"adj", "Adjust stack"}},
        {11, {"a2sp", "A to SP"}},
        {12, {"sp2a", "SP to A"}},
        {13, {"call", "Call procedure"}},
        {14, {"return", "Return from procedure"}},
        {15, {"brz", "Branch if zero"}},
        {16, {"brlz", "Branch if less than zero"}},
        {17, {"br", "Branch unconditional"}},
        {18, {"HALT", "Halt execution"}},
        {19, {"SET", "Set memory location"}}
    };

    void checkMemoryAccess(int32_t addr, const string& operation) const {
        if (addr < 0 || addr >= MEMORY_SIZE) {
            throw runtime_error("Memory access violation during " + operation);
        }
    }

    int32_t signExtend24(int32_t value) const {
        // For ldc instruction, we want the raw value without sign extension
        return value & 0x00FFFFFF;
    }

    void writeTrace(const string& mnemonic, int32_t operand) {
        if (traceFile.is_open()) {
            traceFile << "PC: " << setfill('0') << setw(8) << hex << (PC)
                     << "    SP: " << setfill('0') << setw(8) << hex << SP
                     << "    A: " << setfill('0') << setw(8) << hex << A
                     << "     B: " << setfill('0') << setw(8) << hex << B
                     << "     " << mnemonic << " " << hex << "0x" << operand << endl;
        }
    }

    bool isValidOpcode(int opcode) const {
        return instructions.find(opcode) != instructions.end();
    }

public:
    CPU() : A(0), B(0), PC(0), SP(MAX_STACK), halted(false), lastPC(-1), sameLocationCount(0) {
        memory.resize(MEMORY_SIZE, 0);
    }

    ~CPU() {
        if (traceFile.is_open()) {
            traceFile.close();
        }
    }

    void openTraceFile(const string& filename) {
        string baseFilename = filename;
        size_t lastDot = filename.find_last_of('.');
        if (lastDot != string::npos) {
            baseFilename = filename.substr(0, lastDot);
        }
        
        string traceFilename = baseFilename + ".trace";
        traceFile.open(traceFilename);
        
        if (!traceFile.is_open()) {
            cerr << "Error: Cannot create trace file " << traceFilename << endl;
        }
    }

    void displayISA() const {
        for (const auto& inst : instructions) {
            cout << setw(2) << dec << inst.first << "  "
                 << setw(8) << left << inst.second.first << "  "
                 << inst.second.second << endl;
        }
    }

    bool loadProgram(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Error: Cannot open file " << filename << endl;
            return false;
        }

        int addr = 0;
        int32_t word;
        while (file.read(reinterpret_cast<char*>(&word), sizeof(int32_t))) {
            checkMemoryAccess(addr, "program load");
            memory[addr++] = word;
        }
        
        return true;
    }

    void dumpMemory() const {
        for (int i = 0; i < MEMORY_SIZE; i += 4) {
            if (memory[i] == 0) continue;
            
            int32_t instr = memory[i];
            int opcode = instr & 0xFF;
            int32_t operand = (instr >> 8) & 0xFFFFFF;

            cout << setfill('0') << setw(8) << hex << i << ": "
                 << setfill('0') << setw(8) << hex << instr << "  ";
            
            if (isValidOpcode(opcode)) {
                cout << instructions.at(opcode).first << " 0x" << hex << operand;
            } else {
                cout << "0x" << hex << instr;
            }
            cout << endl;
        }
    }

    bool step(bool trace = false) {
        if (halted) return false;
        
        checkMemoryAccess(PC, "instruction fetch");
        int32_t instruction = memory[PC];
        int opcode = instruction & 0xFF;
        int32_t operand = (instruction >> 8) & 0xFFFFFF;  // Raw operand without sign extension

        if (trace && isValidOpcode(opcode)) {
            writeTrace(instructions.at(opcode).first, operand);
        }

        // Loop detection
        if (PC == lastPC) {
            sameLocationCount++;
            if (sameLocationCount > 2) {
                cout << "Infinite loop detected" << endl;
                halted = true;
                return false;
            }
        } else {
            sameLocationCount = 0;
        }
        lastPC = PC;

        PC++;

        // Execute instruction
        try {
            switch (opcode) {
                case 0:  // ldc - Load constant without sign extension
                    B = A;
                    A = operand;
                    break;
                case 1:  // adc
                    A += signExtend24(operand);
                    break;
                case 2:  // ldl
                    checkMemoryAccess(SP + signExtend24(operand), "load local");
                    B = A;
                    A = memory[SP + signExtend24(operand)];
                    break;
                case 3:  // stl
                    checkMemoryAccess(SP + signExtend24(operand), "store local");
                    memory[SP + signExtend24(operand)] = A;
                    A = B;
                    break;
                case 4:  // ldnl
                    checkMemoryAccess(A + signExtend24(operand), "load non-local");
                    A = memory[A + signExtend24(operand)];
                    break;
                case 5:  // stnl
                    checkMemoryAccess(A + signExtend24(operand), "store non-local");
                    memory[A + signExtend24(operand)] = B;
                    break;
                case 6:  // add
                    A = B + A;
                    break;
                case 7:  // sub
                    A = B - A;
                    break;
                case 8:  // shl
                    A = B << A;
                    break;
                case 9:  // shr
                    A = B >> A;
                    break;
                case 10: // adj
                    SP = SP + signExtend24(operand);
                    break;
                case 11: // a2sp
                    SP = A;
                    A = B;
                    break;
                case 12: // sp2a
                    B = A;
                    A = SP;
                    break;
                case 13: // call
                    B = A;
                    A = PC;
                    PC = PC + signExtend24(operand);
                    break;
                case 14: // return
                    PC = A;
                    A = B;
                    break;
                case 15: // brz
                    if (A == 0) PC = PC + signExtend24(operand);
                    break;
                case 16: // brlz
                    if (A < 0) PC = PC + signExtend24(operand);
                    break;
                case 17: // br
                    PC = PC + signExtend24(operand);
                    break;
                case 18: // HALT
                    halted = true;
                    return false;
                case 19: // SET
                    checkMemoryAccess(A, "SET instruction");
                    memory[A] = operand;
                    A = B;
                    break;
                default:
                    break;
            }
        } catch (const runtime_error& e) {
            cerr << "Runtime error: " << e.what() << endl;
            halted = true;
            return false;
        }
        return true;
    }

    void run(bool trace = false) {
        while (step(trace));
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./emu.exe [option] file.o\n";
        cout << "Options:\n";
        cout << "\t-trace\tshow instruction trace\n";
        cout << "\t-before\tshow memory dump before execution\n";
        cout << "\t-after\tshow memory dump after execution\n";
        cout << "\t-isa\tdisplay ISA\n";
        return 1;
    }

    bool trace = false, before = false, after = false, isa = false;
    string filename;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-trace") trace = true;
        else if (arg == "-before") before = true;
        else if (arg == "-after") after = true;
        else if (arg == "-isa") isa = true;
        else filename = arg;
    }

    CPU cpu;

    if (isa) {
        cpu.displayISA();
        return 0;
    }

    if (!cpu.loadProgram(filename)) {
        return 1;
    }

    if (trace) {
        cpu.openTraceFile(filename);
    }

    if (before) {
        cpu.dumpMemory();
    }

    cpu.run(trace);

    if (after) {
        cpu.dumpMemory();
    }

    return 0;
}