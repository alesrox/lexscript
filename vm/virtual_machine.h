#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define RECURSION_LIMIT 128
#define STACK_SIZE 256
#define MEMORY_SIZE 512

typedef enum {
    ERR_FILE_NOT_FOUND,
    ERR_RECURSION_LIMIT,
    ERR_MEMORY_OUT_OF_BOUNDS,
    ERR_EMPTY_STACK,
    ERR_BAD_TYPE_ARR,
    ERR_OUT_OF_BOUNDS,
    ERR_COUNT 
} ErrorCode;

char* error_messages[ERR_COUNT];

typedef enum {
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    ARRAY_TYPE
} DataType;

typedef struct {
    DataType type;
    uint32_t value;
} DataItem;

typedef struct {
    int size;
    int capacity;
    DataType type;
    uint32_t *items;
} DynamicArray;

typedef struct {
    int pointer;
    int capacity;
    DataItem* data;
} DataSegment;

typedef struct {
    uint8_t opcode;
    DataItem arg;
} Instruction;

typedef struct {
    DataSegment locals;
    int return_address;
} Frame;

typedef struct {
    int pc; // program count
    int sp; // stack pointer
    int fp; // frame pointer
    int asp; // array storage pointer
    Frame frames[RECURSION_LIMIT];
    DataItem stack[STACK_SIZE];
    Instruction* memory;
    DataSegment data_segment;
    DynamicArray array_storage[1024];
} VM;

// Virtual Machine Control
int load_program(VM *vm, const char *filename);
void throw_error(char* msg);
void run(VM *vm, int size);

// VM Core
DataItem pop(VM *vm);
void push(VM *vm, DataItem value);
void store_data(DataSegment *ds, int address, DataItem item);
void syscall(VM *vm, int arg);
void sub_print(DataItem item);
void sup_print(VM* vm, DataItem element);

// ALU Core
uint32_t format_float(float value);
float extract_float(DataItem item);
float float_mod(float a, float b);
int logic_unit(VM* vm, DataItem left, DataItem right, uint8_t op);
float float_alu(VM* vm, DataItem left, DataItem right, uint8_t op);
int int_alu(VM* vm, DataItem left, DataItem right, uint8_t op);
DataItem alu(VM* vm, DataItem left, DataItem right, uint8_t op);

// Arrays Utils
void create_array(DynamicArray *array, int initial_capacity);
void resize_array(DynamicArray *array, int new_capacity);
void append_array(DynamicArray *array, uint32_t value);
void remove_at(DynamicArray* array, int index);
void remove_last(DynamicArray *array);

// Stings Utils
void convert_int_to_str(DynamicArray *arr, uint32_t value);
void convert_float_to_str(DynamicArray *arr, uint32_t value);
void convert_list_to_str(VM* vm, DynamicArray* arr, DynamicArray list);
