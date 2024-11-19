#include "virtual_machine.h"

void create_array(DynamicArray *array, int initial_capacity) {
    array->items = malloc(initial_capacity * sizeof(uint32_t));
    array->size = 0;
    array->capacity = initial_capacity;
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
        append_array(arr, 45); // -
        value = -value;
    }

    do {
        digits[digit_count++] = value % 10;
        value /= 10;
    } while (value > 0);


    for (int i = digit_count - 1; i >= 0; i--)
        append_array(arr, 48 + digits[i]);
}

void convert_float_to_str(DynamicArray *arr, uint32_t float_num) {
    DataItem aux = {FLOAT_TYPE, float_num};
    float value = extract_float(aux);

    if (value < 0) {
        append_array(arr, 45); // -
        value = -value;
    }

    int integer_part = (int) value;
    float fractional_part = value - integer_part;

    convert_int_to_str(arr, integer_part);

    if (fractional_part > 0) {
        append_array(arr, 46); // .
        // Limit to 6 decimals
        for (int i = 0; i < 8 && fractional_part > 0.0001; i++) { 
            fractional_part *= 10;
            int digit = (int) fractional_part;
            append_array(arr, 48 + digit);
            fractional_part -= digit;
        }
    }
}

void convert_list_to_str(VM* vm, DynamicArray* arr, DynamicArray list) {
    append_array(arr, (uint32_t) '[');

    void (*convert_func)(DynamicArray*, uint32_t);
    convert_func = (list.type == INT_TYPE) ? convert_int_to_str : convert_float_to_str;

    for (int i = 0; i < list.size; i++) {
        (list.type == ARRAY_TYPE) ? convert_list_to_str(vm, arr, vm->array_storage[list.items[i]]) : convert_func(arr, list.items[i]);
        if (i != list.size - 1) append_array(arr, (uint32_t) ',');
    }

    append_array(arr, (uint32_t) ']');
}

void list_append(VM* vm, DataItem obj) {
    DataItem aux = pop(vm);

    if (vm->array_storage[obj.value].type != aux.type)
        throw_error(error_messages[ERR_BAD_TYPE_ARR]);

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

    if (from < 0 || from > arr.size || to < 0 || to > arr.size)
        throw_error(error_messages[ERR_OUT_OF_BOUNDS]);

    create_array(&vm->array_storage[sliced_arr.value], 4);
    vm->array_storage[sliced_arr.value].type = arr.type;
    for (int i = from; i < to; i++)
        append_array(&vm->array_storage[sliced_arr.value], arr.items[i]);

    push(vm, sliced_arr);
}

void list_map(VM* vm, DataItem obj) {
    // TODO
    DataItem map_func = pop(vm);
    push(vm, obj);
}

void list_filter(VM* vm, DataItem obj) {
    // TODO
    DataItem filter_func = pop(vm);
    push(vm, obj);
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

void list_lower(VM* vm, DataItem obj) {
    if (vm->array_storage[obj.value].type != CHAR_TYPE) {
        push(vm, obj);
        return;
    }

    DataItem arr = {ARRAY_TYPE, vm->asp++};
    create_array(&vm->array_storage[arr.value], 4);
    vm->array_storage[arr.value].type = CHAR_TYPE;

    for (int i = 0; i < vm->array_storage[obj.value].size; i++) {
        int value = (int) vm->array_storage[obj.value].items[i];
        
        if (value > 96 && value < 123)
            append_array(&vm->array_storage[arr.value], value - 32);
        else
            append_array(&vm->array_storage[arr.value], value);
    }

    push(vm, arr);
}

void list_upper(VM* vm, DataItem obj) {
    if (vm->array_storage[obj.value].type != CHAR_TYPE) {
        push(vm, obj);
        return;
    }

    DataItem arr = {ARRAY_TYPE, vm->asp++};
    create_array(&vm->array_storage[arr.value], 4);
    vm->array_storage[arr.value].type = CHAR_TYPE;

    for (int i = 0; i < vm->array_storage[obj.value].size; i++) {
        int value = (int) vm->array_storage[obj.value].items[i];
        
        if (value > 64 && value < 91)
            append_array(&vm->array_storage[arr.value], value + 32);
        else
            append_array(&vm->array_storage[arr.value], value);
    }

    push(vm, arr);
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
};

void objcall(VM *vm, int arg) {
    if (arg > -1 && arg < 12) objfuncs[arg](vm, pop(vm));
    else printf("Unknown objcall: %d\n", arg);
}
