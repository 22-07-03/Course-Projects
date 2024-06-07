#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>
using namespace std;

class FetchBuffer {
public:
    uint8_t instructionNumber;
};

class DecodeBuffer {
public:
    uint8_t instructionNumber;
    uint16_t instructionRegister;
};

class ExecuteBuffer {
public:
    uint8_t instructionNumber;
    uint8_t operation;
    uint8_t destinationRegister;
    int8_t signedImmediate;
    uint8_t unsignedImmediate;
    int8_t operandA;
    int8_t operandB;
    uint16_t instructionRegister;
};

class MemoryBuffer {
public:
    uint8_t operation;
    uint8_t instructionNumber;
    uint8_t destinationRegister;
    int8_t aluOutput;
    int8_t operandB;
    uint16_t instructionRegister;
};

class WriteBackBuffer {
public:
    uint8_t instructionNumber;
    int8_t aluOutput;
    int8_t loadMemoryData;
    uint8_t destinationRegister;
    uint8_t operation;
    uint16_t instructionRegister;
};

int main() {
    int8_t dataCache[256];
    uint8_t instructionCache[256];
    int8_t registerFile[16];

    int totalInstructions = 0;
    int arithmeticInstructions = 0;
    int logicalInstructions = 0;
    int shiftInstructions = 0;
    int memoryInstructions = 0;
    int loadImmediateInstructions = 0;
    int controlInstructions = 0;
    int haltInstructions = 0;

    int totalStalls = 0;
    int dataStalls = 0;
    int controlStalls = 0;

    ifstream instructionCacheRead("input/ICache.txt");
    ifstream dataCacheRead("input/DCache.txt");
    ifstream registerFileRead("input/RF.txt");

    for (int i = 0; i < 256; i++) {
        string temp;
        instructionCacheRead >> temp;
        instructionCache[i] = static_cast<uint8_t>(stoi(temp, nullptr, 16));
    }

    for (int i = 0; i < 256; i++) {
        string temp;
        dataCacheRead >> temp;
        dataCache[i] = static_cast<int8_t>(stoi(temp, nullptr, 16));
    }

    for (int i = 0; i < 16; i++) {
        string temp;
        registerFileRead >> temp;
        registerFile[i] = static_cast<int8_t>(stoi(temp, nullptr, 16));
    }

    int clockCycles = 0;

    FetchBuffer fetchBuffer;
    DecodeBuffer decodeBuffer;
    ExecuteBuffer executeBuffer;
    MemoryBuffer memoryBuffer;
    WriteBackBuffer writeBackBuffer;

    int programCounter = 0;
    bool fetch = true;
    bool decode = false;
    bool execute = false;
    bool memory = false;
    bool writeBack = false;
    int stallCause = -1;
    int hazards[16] = {0};

    while (true) {
        clockCycles++;

        if (stallCause != 3) {
            fetch = true;
        }

        if (writeBack) {
            if (writeBackBuffer.operation == 15) {
                break;
            }
            if (writeBackBuffer.operation <= 10) {
                registerFile[writeBackBuffer.destinationRegister] = writeBackBuffer.aluOutput;
                hazards[writeBackBuffer.destinationRegister]--;
            }
            if (writeBackBuffer.operation == 11) {
                registerFile[writeBackBuffer.destinationRegister] = writeBackBuffer.loadMemoryData;
                hazards[writeBackBuffer.destinationRegister]--;
            }
        }

        if (memory) {
            writeBack = true;
            writeBackBuffer.instructionRegister = memoryBuffer.instructionRegister;
            writeBackBuffer.destinationRegister = memoryBuffer.destinationRegister;
            writeBackBuffer.aluOutput = memoryBuffer.aluOutput;
            writeBackBuffer.operation = memoryBuffer.operation;
            if (memoryBuffer.operation == 11) {
                writeBackBuffer.loadMemoryData = dataCache[memoryBuffer.aluOutput];
            }
            if (memoryBuffer.operation == 12) {
                dataCache[memoryBuffer.aluOutput] = memoryBuffer.operandB;
            }
        } else {
            writeBack = false;
        }

        if (execute) {
            memory = true;
            memoryBuffer.instructionRegister = executeBuffer.instructionRegister;
            memoryBuffer.destinationRegister = executeBuffer.destinationRegister;
            memoryBuffer.operation = executeBuffer.operation;
            uint8_t operation = executeBuffer.operation;
            int8_t aluOutput;

            totalInstructions++;

            if (operation == 0) {
                aluOutput = executeBuffer.operandA + executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                arithmeticInstructions++;
            } else if (operation == 1) {
                aluOutput = executeBuffer.operandA - executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                arithmeticInstructions++;
            } else if (operation == 2) {
                aluOutput = executeBuffer.operandA * executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                arithmeticInstructions++;
            } else if (operation == 3) {
                aluOutput = executeBuffer.operandA + 1;
                memoryBuffer.aluOutput = aluOutput;
                arithmeticInstructions++;
            } else if (operation == 4) {
                aluOutput = executeBuffer.operandA & executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                logicalInstructions++;
            } else if (operation == 5) {
                aluOutput = executeBuffer.operandA | executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                logicalInstructions++;
            } else if (operation == 6) {
                aluOutput = executeBuffer.operandA ^ executeBuffer.operandB;
                memoryBuffer.aluOutput = aluOutput;
                logicalInstructions++;
            } else if (operation == 7) {
                aluOutput = ~executeBuffer.operandA;
                memoryBuffer.aluOutput = aluOutput;
                logicalInstructions++;
            } else if (operation == 8) {
                aluOutput = executeBuffer.operandA << executeBuffer.unsignedImmediate;
                memoryBuffer.aluOutput = aluOutput;
                shiftInstructions++;
            } else if (operation == 9) {
                uint8_t temp = static_cast<uint8_t>(executeBuffer.operandA);
                temp = temp >> executeBuffer.unsignedImmediate;
                aluOutput = static_cast<int8_t>(temp);
                memoryBuffer.aluOutput = aluOutput;
                shiftInstructions++;
            } else if (operation == 10) {
                aluOutput = executeBuffer.signedImmediate;
                memoryBuffer.aluOutput = aluOutput;
                loadImmediateInstructions++;
            } else if (operation == 11) {
                aluOutput = executeBuffer.operandA + executeBuffer.signedImmediate;
                memoryBuffer.aluOutput = aluOutput;
                memoryInstructions++;
            } else if (operation == 12) {
                aluOutput = executeBuffer.operandA + executeBuffer.signedImmediate;
                memoryBuffer.aluOutput = aluOutput;
                memoryInstructions++;
                memoryBuffer.operandB = executeBuffer.operandB;
            } else if (operation == 13) {
                programCounter += 2 * executeBuffer.signedImmediate;
                fetch = false;
                controlInstructions++;
            } else if (operation == 14) {
                if (executeBuffer.operandA == 0) {
                    programCounter += 2 * executeBuffer.signedImmediate;
                }
                fetch = false;
                controlInstructions++;
            } else if (operation == 15) {
                decode = false;
                haltInstructions++;
            }
        } else {
            memory = false;
        }

        if (decode) {
            execute = true;

            executeBuffer.instructionRegister = decodeBuffer.instructionRegister;
            uint16_t instruction = decodeBuffer.instructionRegister;
            uint8_t operation = instruction >> 12;

            executeBuffer.instructionNumber = decodeBuffer.instructionNumber;
            executeBuffer.operation = operation;
            uint8_t destinationRegister;
            uint8_t sourceRegister1;
            uint8_t sourceRegister2;
            int8_t signedImmediate;
            uint8_t unsignedImmediate;
            int8_t operandA;
            int8_t operandB;

            if (operation == 0 || operation == 1 || operation == 2 || operation == 4 || operation == 5 || operation == 6) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                sourceRegister1 = (instruction >> 4) & 0b00001111;
                sourceRegister2 = instruction & 0b00001111;
                if (hazards[sourceRegister1] != 0 || hazards[sourceRegister2] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    hazards[destinationRegister]++;
                    operandA = registerFile[sourceRegister1];
                    operandB = registerFile[sourceRegister2];
                    executeBuffer.operandA = operandA;
                    executeBuffer.operandB = operandB;
                    executeBuffer.destinationRegister = destinationRegister;
                }
            } else if (operation == 3) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                sourceRegister1 = destinationRegister;
                if (hazards[sourceRegister1] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    hazards[destinationRegister]++;
                    operandA = registerFile[sourceRegister1];
                    executeBuffer.operandA = operandA;
                    executeBuffer.destinationRegister = destinationRegister;
                }
            } else if (operation == 7) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                sourceRegister1 = (instruction >> 4) & 0b00001111;
                if (hazards[sourceRegister1] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    hazards[destinationRegister]++;
                    operandA = registerFile[sourceRegister1];
                    executeBuffer.operandA = operandA;
                    executeBuffer.destinationRegister = destinationRegister;
                }
            } else if (operation == 8 || operation == 9) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                sourceRegister1 = (instruction >> 4) & 0b00001111;
                unsignedImmediate = instruction & 0b00001111;
                if (hazards[sourceRegister1] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    hazards[destinationRegister]++;
                    operandA = registerFile[sourceRegister1];
                    executeBuffer.operandA = operandA;
                    executeBuffer.unsignedImmediate = unsignedImmediate;
                    executeBuffer.destinationRegister = destinationRegister;
                }
            } else if (operation == 10) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                signedImmediate = instruction & 0b11111111;
                executeBuffer.signedImmediate = signedImmediate;
                executeBuffer.destinationRegister = destinationRegister;
                hazards[destinationRegister]++;
            } else if (operation == 11) {
                destinationRegister = (instruction >> 8) & 0b00001111;
                sourceRegister1 = (instruction >> 4) & 0b00001111;
                signedImmediate = instruction & 0b11111111;
                if (hazards[sourceRegister1] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    hazards[destinationRegister]++;
                    operandA = registerFile[sourceRegister1];
                    executeBuffer.operandA = operandA;
                    executeBuffer.signedImmediate = signedImmediate;
                    executeBuffer.destinationRegister = destinationRegister;
                }
            } else if (operation == 12) {
                sourceRegister1 = (instruction >> 8) & 0b00001111;
                sourceRegister2 = (instruction >> 4) & 0b00001111;
                signedImmediate = instruction & 0b11111111;
                if (hazards[sourceRegister1] != 0 || hazards[sourceRegister2] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    operandA = registerFile[sourceRegister1];
                    operandB = registerFile[sourceRegister2];
                    executeBuffer.operandA = operandA;
                    executeBuffer.operandB = operandB;
                    executeBuffer.signedImmediate = signedImmediate;
                }
            } else if (operation == 13 || operation == 14) {
                sourceRegister1 = (instruction >> 8) & 0b00001111;
                signedImmediate = instruction & 0b11111111;
                if (hazards[sourceRegister1] != 0) {
                    stallCause = 2;
                    execute = false;
                    fetch = false;
                } else {
                    operandA = registerFile[sourceRegister1];
                    executeBuffer.operandA = operandA;
                    executeBuffer.signedImmediate = signedImmediate;
                    stallCause = 3;
                }
            } else if (operation == 15) {
                fetch = false;
            }

            executeBuffer.destinationRegister = destinationRegister;
            executeBuffer.sourceRegister1 = sourceRegister1;
            executeBuffer.sourceRegister2 = sourceRegister2;
            executeBuffer.signedImmediate = signedImmediate;
            executeBuffer.unsignedImmediate = unsignedImmediate;
            executeBuffer.operandA = operandA;
            executeBuffer.operandB = operandB;
            executeBuffer.instructionRegister = instruction;
            executeBuffer.instructionNumber = decodeBuffer.instructionNumber;
        } else {
            execute = false;
        }

        if (fetch) {
            decode = true;
            decodeBuffer.instructionRegister = (instructionCache[programCounter + 1] << 8) | instructionCache[programCounter];
            decodeBuffer.instructionNumber = fetchBuffer.instructionNumber;
            fetchBuffer.instructionNumber = programCounter / 2;
            programCounter += 2;
        } else {
            decode = false;
        }
    }

    // Output results
    cout << "Total Instructions: " << totalInstructions << endl;
    cout << "Arithmetic Instructions: " << arithmeticInstructions << endl;
    cout << "Logical Instructions: " << logicalInstructions << endl;
    cout << "Shift Instructions: " << shiftInstructions << endl;
    cout << "Memory Instructions: " << memoryInstructions << endl;
    cout << "Load Immediate Instructions: " << loadImmediateInstructions << endl;
    cout << "Control Instructions: " << controlInstructions << endl;
    cout << "Halt Instructions: " << haltInstructions << endl;
    cout << "Total Stalls: " << totalStalls << endl;
    cout << "Data Stalls: " << dataStalls << endl;
    cout << "Control Stalls: " << controlStalls << endl;
    cout << "Clock Cycles: " << clockCycles << endl;

    // Write updated data cache and register file
    ofstream dataCacheWrite("output/DCache.txt");
    for (int i = 0; i < 256; i++) {
        dataCacheWrite << hex << setw(2) << setfill('0') << (0xff & dataCache[i]) << " ";
    }
    dataCacheWrite.close();

    ofstream registerFileWrite("output/RF.txt");
    for (int i = 0; i < 16; i++) {
        registerFileWrite << hex << setw(2) << setfill('0') << (0xff & registerFile[i]) << " ";
    }
    registerFileWrite.close();

    return 0;
}
