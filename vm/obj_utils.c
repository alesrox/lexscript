#include "virtual_machine.h"

void init_array(DynamicArray *array, int initial_capacity) {
    array->items = malloc(initial_capacity * sizeof(uint32_t));
    array->capacity = initial_capacity;
    array->type = UNASSIGNED_TYPE;
    array->size = 0;
}

void array_assigment(VM* vm, int address, int item) {
    int arr_addres_type = vm->array_storage[address].type == ARRAY_TYPE;
    int arr_item_type = vm->array_storage[item].type == ARRAY_TYPE;

    if (arr_addres_type && arr_item_type) 
        array_assigment(vm, vm->array_storage[address].items[0], vm->array_storage[item].items[0]);
    else if (arr_addres_type + arr_item_type == 0 || arr_item_type) 
        if (vm->array_storage[address].type == UNASSIGNED_TYPE) vm->array_storage[item].type = INT_TYPE;
        else vm->array_storage[item].type = vm->array_storage[address].type;
    else if (arr_addres_type) 
        throw_error(error_messages[ERR_BAD_TYPE]);
}

void resize_array(DynamicArray *array, int new_capacity) {
    array->items = realloc(array->items, new_capacity * sizeof(int));
    array->capacity = new_capacity;
}

void append_array(DynamicArray *array, uint32_t value) {
    if (array->size == array->capacity)
        resize_array(array, array->capacity * 2);

    array->items[array->size++] = value;
}

void remove_at(DynamicArray* array, int index) {
    if (index < 0 || index >= array->size) {
        throw_error(error_messages[ERR_OUT_OF_BOUNDS]);
    }

    for (int i = index; i < array->size - 1; i++) {
        array->items[i] = array->items[i + 1];
    }

    array->size--;

    if (array->size > 0 && array->size < array->capacity / 4) {
        resize_array(array, array->capacity / 2);
    }
}

void remove_last(DynamicArray *array) {
    if (array->size > 0) {
        array->size--;
        if (array->size > 0 && array->size < array->capacity / 4)
            resize_array(array, array->capacity / 2);  // Reducir capacidad
    }
}

// String utils 
void convert_int_to_str(DynamicArray *arr, uint32_t integer) {
    int value = integer;
    int digits[10], digit_count = 0;

    if (value < 0) {
        uint32_t temp = 0;
        temp |= ((uint32_t) 45);
        append_array(arr, temp);
        value = -value;
    }

    do {
        digits[digit_count++] = value % 10;
        value /= 10;
    } while (value > 0);

    uint32_t temp = 0; 
    int shift = 0;
    for (int i = digit_count - 1; i >= 0; i--) {
        temp |= ((uint32_t)(48 + digits[i])) << shift;
        shift += 8;

        if (shift == 32) {
            append_array(arr, temp);
            temp = 0; shift = 0;
        }
    }

    if (shift != 0) append_array(arr, temp);
}

void convert_float_to_str(DynamicArray *arr, uint32_t float_num) {
    DataItem aux = {FLOAT_TYPE, float_num};
    float value = extract_float(aux);

    if (value < 0) {
        uint32_t temp = 0;
        temp |= ((uint32_t)45) << 0; // '-' en el último byte
        append_array(arr, temp);
        value = -value;
    }

    int integer_part = (int)value;
    float fractional_part = value - integer_part;

    convert_int_to_str(arr, integer_part);

    if (fractional_part > 0) {
        uint32_t temp = 0;
        int shift = 0;

        append_array(arr, 46); // '.'

        for (int i = 0; i < 8 && fractional_part > 0.0001; i++) {
            fractional_part *= 10;
            int digit = (int)fractional_part;

            temp |= ((uint32_t)(48 + digit)) << shift;
            shift += 8;

            if (shift == 32) {
                append_array(arr, temp);
                temp = 0; 
                shift = 0;
            }

            fractional_part -= digit;
        }

        if (shift != 0) append_array(arr, temp);
    }
}

// List functions
void list_append(VM* vm, DataItem obj) {
    DataItem aux = pop(vm);

    if (vm->array_storage[obj.value].type == UNASSIGNED_TYPE)
        vm->array_storage[obj.value].type = aux.type;
    else if (vm->array_storage[obj.value].type != aux.type) {
        if (vm->array_storage[obj.value].type < CHAR_TYPE) {
            if (aux.type == FLOAT_TYPE) aux.value = (int) extract_float(aux);
            else if (vm->array_storage[obj.value].type == FLOAT_TYPE) aux.value = format_float(aux.value);
            else throw_error(error_messages[ERR_BAD_TYPE]);
        } else throw_error(error_messages[ERR_BAD_TYPE]);
    }

    append_array(&vm->array_storage[obj.value], aux.value);
}

void list_size(VM* vm, DataItem obj) {
    DataItem size_value;
    size_value.type = INT_TYPE;
    size_value.value = vm->array_storage[obj.value].size;
    push(vm, size_value);
}

void list_remove(VM* vm, DataItem obj) {
    DataItem aux = pop(vm);
    remove_at(&vm->array_storage[obj.value], aux.value);
}

void list_pop(VM* vm, DataItem obj) {
    remove_last(&vm->array_storage[obj.value]);
}

void list_empty(VM* vm, DataItem obj) {
    DataItem result;
    result.type = INT_TYPE;
    result.value = (vm->array_storage[obj.value].size == 0) ? 1 : 0;
    push(vm, result);
}

void list_slice(VM* vm, DataItem obj) {
    int to = pop(vm).value;
    int from = pop(vm).value;
    DynamicArray arr = vm->array_storage[obj.value];
    DataItem sliced_arr = {ARRAY_TYPE, vm->asp++};

    int size = (arr.type == CHAR_TYPE) ? arr.size * 4 : arr.size;

    if (from < 0 || from > size || to < 0 || to > size)
        throw_error(error_messages[ERR_OUT_OF_BOUNDS]);

    init_array(&vm->array_storage[sliced_arr.value], 4);
    vm->array_storage[sliced_arr.value].type = arr.type;
    if (arr.type == CHAR_TYPE) {
        uint32_t current_value = 0; int char_count = 0;
        for (int i = from; i < to; i++) {
            int current_uint32_index = i / 4;
            int char_index_in_uint32 = i % 4;
            uint32_t value = arr.items[current_uint32_index];
            uint8_t char_value = (value >> (8 * char_index_in_uint32)) & 0xFF;
            current_value |= (char_value << (8 * char_count));
            char_count++;

            if (char_count == 4) {
                append_array(&vm->array_storage[sliced_arr.value], current_value);
                current_value = 0; char_count = 0;
            }
        }

        if (char_count > 0) append_array(&vm->array_storage[sliced_arr.value], current_value);
    } else {
        for (int i = from; i < to; i++)
            append_array(&vm->array_storage[sliced_arr.value], arr.items[i]);
    }

    push(vm, sliced_arr);
}

void list_map(VM* vm, DataItem obj) {
    DataItem mapped_arr = {ARRAY_TYPE, vm->asp++};
    DataItem map_func = pop(vm);
    DynamicArray arr = vm->array_storage[obj.value];

    init_array(&vm->array_storage[mapped_arr.value], arr.capacity);
    vm->array_storage[mapped_arr.value].type = arr.type;
    for (int i = 0; i < arr.size; i++) {
        DataItem arg = {arr.type, arr.items[i]};
        push(vm, arg);
        run_function(vm, map_func.value);
        append_array(&vm->array_storage[mapped_arr.value], pop(vm).value);
    }

    push(vm, mapped_arr);
}

void list_filter(VM* vm, DataItem obj) {
    DataItem filtered_arr = {ARRAY_TYPE, vm->asp++};
    DataItem map_func = pop(vm);
    DynamicArray arr = vm->array_storage[obj.value];

    init_array(&vm->array_storage[filtered_arr.value], arr.capacity);
    vm->array_storage[filtered_arr.value].type = arr.type;
    for (int i = 0; i < arr.size; i++) {
        DataItem arg = {arr.type, arr.items[i]};
        push(vm, arg);
        run_function(vm, map_func.value);
        DataItem result = pop(vm);
        if (result.value != 0)
            append_array(&vm->array_storage[filtered_arr.value], arr.items[i]);
    }

    push(vm, filtered_arr);
}

void list_min(VM* vm, DataItem obj) {
    DynamicArray arr = vm->array_storage[obj.value];
    int min = (int) arr.items[0];

    for (int i = 0; i < arr.size; i++)
        if (min > (int) arr.items[i]) min = arr.items[i];

    push(vm, (DataItem){arr.type, min});
}

void list_max(VM* vm, DataItem obj) {
    DynamicArray arr = vm->array_storage[obj.value];
    int max = (int) arr.items[0];

    for (int i = 0; i < arr.size; i++)
        if (max < (int) arr.items[i]) max = arr.items[i];

    push(vm, (DataItem){arr.type, max});
}

void list_upper(VM* vm, DataItem obj) {
    if (vm->array_storage[obj.value].type != CHAR_TYPE) {
        push(vm, obj);
        return;
    }

    DataItem arr = {ARRAY_TYPE, vm->asp++};
    init_array(&vm->array_storage[arr.value], 4);
    vm->array_storage[arr.value].type = CHAR_TYPE;

    for (int i = 0; i < vm->array_storage[obj.value].size; i++) {
        uint32_t value = vm->array_storage[obj.value].items[i];
        for (int j = 0; j < 4; j++) {
            uint8_t char_value = (value >> (8 * j)) & 0xFF;
            if (char_value >= 'a' && char_value <= 'z') char_value -= 32;
            value = (value & ~(0xFF << (8 * j))) | (char_value << (8 * j));
        }
        append_array(&vm->array_storage[arr.value], value);
    }

    push(vm, arr);
}

void list_lower(VM* vm, DataItem obj) {
    if (vm->array_storage[obj.value].type != CHAR_TYPE) {
        push(vm, obj);
        return;
    }

    DataItem arr = {ARRAY_TYPE, vm->asp++};
    init_array(&vm->array_storage[arr.value], 4);
    vm->array_storage[arr.value].type = CHAR_TYPE;

    for (int i = 0; i < vm->array_storage[obj.value].size; i++) {
        uint32_t value = vm->array_storage[obj.value].items[i];
        for (int j = 0; j < 4; j++) {
            uint8_t char_value = (value >> (8 * j)) & 0xFF;
            if (char_value >= 'A' && char_value <= 'Z') char_value += 32;
            value = (value & ~(0xFF << (8 * j))) | (char_value << (8 * j));
        }
        append_array(&vm->array_storage[arr.value], value);
    }

    push(vm, arr);
}

void list_to_string(VM* vm, DataItem obj) {
    DataItem arr_pos = {ARRAY_TYPE, vm->asp++};
    DynamicArray* converterd_arr = &vm->array_storage[arr_pos.value];
    init_array(converterd_arr, 4);
    converterd_arr->type = CHAR_TYPE;

    DynamicArray* arr = &vm->array_storage[obj.value];
    if (arr->type == CHAR_TYPE) {
        push(vm, obj);
        return;
    }

    append_array(converterd_arr, '[');

    void (*convert_func)(DynamicArray*, uint32_t);
    convert_func = (arr->type == INT_TYPE) ? convert_int_to_str : convert_float_to_str;

    for (int i = 0; i < arr->size; i++) {
        if (arr->type == ARRAY_TYPE) {
            list_to_string(vm, (DataItem){ARRAY_TYPE, arr->items[i]});
            DynamicArray* aux = &vm->array_storage[pop(vm).value];

            for (int j = 0; j < aux->size; j++)
                append_array(converterd_arr, aux->items[j]);
            
            vm->asp--;
        } else {
            convert_func(converterd_arr, arr->items[i]);
        }

        if (i != arr->size - 1) 
            append_array(converterd_arr, ',');
    }

    append_array(converterd_arr, ']');
    push(vm, arr_pos);
}

void (*objfuncs[])(VM* vm, DataItem obj) = {
    list_append,
    list_size,
    list_remove,
    list_pop,
    list_empty,
    list_slice,
    list_map,
    list_filter,
    list_min,
    list_max,
    list_lower,
    list_upper,
    list_to_string,
};

void objcall(VM *vm, int arg) {
    DataItem obj = pop(vm);

    if (arg > -1 && arg < 13) objfuncs[arg](vm, obj);
    else printf("Unknown objcall: %d\n", arg);
}
