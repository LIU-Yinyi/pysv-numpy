#include "bridge.hh"
#include "register.hh"

#define BITS_PER_BYTE 8
#define ELEMENT_SIZE_BITS(arr) (svHigh(arr, 0) - svLow(arr, 0) + 1)

extern const PyFileFuncVec pyfilefunc_reg;

typedef std::map<PyFileFunc, PyObject*> PyFuncMap;
PyFuncMap pyfunc_map;

struct iterator_t {
    int low;
    int high;
    int cur;
};

void init_python_env() {
    Py_Initialize();
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));

    for(const auto &reg_pair : pyfilefunc_reg) {
        std::string pyfile_fullpath = std::string("pyfunc.") + reg_pair.first;
        PyObject *pName = PyUnicode_DecodeFSDefault(pyfile_fullpath.c_str());
        PyObject *payload = PyImport_Import(pName);
        if (!payload) {
            std::cout << "[ERROR] Failed to Load Python Func: " << reg_pair.first << "." << reg_pair.second << std::endl;
            exit(-1);
        }
        PyObject *callable = PyObject_GetAttrString(payload, reg_pair.second.c_str());
        pyfunc_map[std::pair<std::string, std::string>(reg_pair.first, reg_pair.second)] = callable;

        Py_DecRef(pName);
        Py_DecRef(payload);
    }
}

void deinit_python_env() {
    for(const auto& pyfunc : pyfunc_map) {
        Py_DecRef(pyfunc.second);
    }
    Py_Finalize();
}

static PyObject *transform_bytes_into_list(uint8_t *ptr, size_t length) {
    PyObject *list = PyList_New(0);
    for (int i = 0; i < length; i++) {
        PyList_Append(list, PyLong_FromLong(*(ptr + i)));
    }
    return list;
}

static uint8_t *transform_list_into_bytes(PyObject *obj, size_t *length) {
    uint8_t *bytes = NULL;
    *length = 0;

    if(PyList_Check(obj)) {
        *length = (size_t)PyList_Size(obj);
        bytes = new uint8_t[*length];
        memset(bytes, 0, *length);
        for(auto i = 0; i < *length; i++) {
            PyObject *item = PyList_GetItem(obj, i);
            bytes[i] = (uint8_t)PyLong_AsLong(item);
        }
    }
    return bytes;
}

static PyObject *get_array_size(svOpenArrayHandle arr) {
    PyObject *list = PyList_New(0);

    int dim = svDimensions(arr);
    assert(dim != 0);

    for (int d = 1; d <= dim; d++) {
        auto length = svHigh(arr, d) - svLow(arr, d) + 1;
        PyList_Append(list, PyLong_FromLong(length));
    }
    
    return list;
}

static PyObject *recursive_get(struct iterator_t *iters, int last_layer, int layer, svOpenArrayHandle arr) {
    assert(layer <= last_layer && layer >= 0);
    struct iterator_t *iter = &iters[layer];
    PyObject *list = PyList_New(0);

    if (layer != last_layer) {
        for (iter->cur = iter->low; iter->cur <= iter->high; iter->cur++) {
            PyList_Append(list, recursive_get(iters, last_layer, layer + 1, arr));
        }
    } else {
        for (iter->cur = iter->low; iter->cur <= iter->high; iter->cur++) {
            uint8_t *ptr = NULL;
            switch (last_layer) {
                case 0:
                    ptr = (uint8_t *)svGetArrElemPtr1(arr, iters[0].cur);
                    break;
                case 1:
                    ptr = (uint8_t *)svGetArrElemPtr2(arr, iters[0].cur, iters[1].cur);
                    break;
                case 2:
                    ptr = (uint8_t *)svGetArrElemPtr3(arr, iters[0].cur, iters[1].cur, iters[2].cur);
                    break;
            }
            if (ptr != NULL) {
                /* The dimension 0 will always be the length of your basic element. */
                PyList_Append(list, transform_bytes_into_list(ptr, (ELEMENT_SIZE_BITS(arr) + BITS_PER_BYTE - 1) / BITS_PER_BYTE));
            }
        }
    }
    return list;
}

static void recursive_set(struct iterator_t *iters, int last_layer, int layer, svOpenArrayHandle arr, PyObject *obj) {
    assert(layer <= last_layer && layer >= 0);
    struct iterator_t *iter = &iters[layer];
    
    if (layer != last_layer) {
        for (iter->cur = iter->low; iter->cur <= iter->high; iter->cur++) {
            PyObject *item = PyList_GetItem(obj, iter->cur - iter->low);
            recursive_set(iters, last_layer, layer + 1, arr, item);
        }
    } else {
        for (iter->cur = iter->low; iter->cur <= iter->high; iter->cur++) {
            svBitVecVal *val = NULL;
            size_t *length = new size_t;
            PyObject *item = PyList_GetItem(obj, iter->cur - iter->low);
            switch (last_layer) {
                case 0:
                    val = (svBitVecVal *)transform_list_into_bytes(item, length);
                    svPutBitArrElem1VecVal(arr, val, iters[0].cur);
                    break;
                case 1:
                    val = (svBitVecVal *)transform_list_into_bytes(item, length);
                    svPutBitArrElem2VecVal(arr, val, iters[0].cur, iters[1].cur);
                    break;
                case 2:
                    val = (svBitVecVal *)transform_list_into_bytes(item, length);
                    svPutBitArrElem3VecVal(arr, val, iters[0].cur, iters[1].cur, iters[2].cur);
                    break;
            }
        }
    }
}

void array_handle(char *pyfilename, char *pyfuncname, svOpenArrayHandle arr_in, svOpenArrayHandle arr_out) {
    // obtain the information of dimensions
    int dim_in = svDimensions(arr_in);
    assert(dim_in != 0);

    int dim_out = svDimensions(arr_out);
    assert(dim_out != 0);

    // configure the bound of the iterators
    struct iterator_t *iters_in = (struct iterator_t *)malloc(sizeof(struct iterator_t) * dim_in);
    memset(iters_in, 0, sizeof(struct iterator_t) * dim_in);
    for (int d = dim_in; d > 0; d--) {
        struct iterator_t *ptr = &iters_in[d - 1];
        ptr->cur = ptr->low = svLow(arr_in, d);
        ptr->high = svHigh(arr_in, d);
    }

    struct iterator_t *iters_out = (struct iterator_t *)malloc(sizeof(struct iterator_t) * dim_out);
    memset(iters_out, 0, sizeof(struct iterator_t) * dim_out);
    for (int d = dim_out; d > 0; d--) {
        struct iterator_t *ptr = &iters_out[d - 1];
        ptr->cur = ptr->low = svLow(arr_out, d);
        ptr->high = svHigh(arr_out, d);
    }

    // match the array and callable
    PyObject *list_in = recursive_get(iters_in, dim_in - 1, 0, arr_in);
    PyObject *list_out = get_array_size(arr_out);

    PyObject *callable = pyfunc_map[PyFileFunc(std::string(pyfilename), std::string(pyfuncname))];
    if (callable && PyCallable_Check(callable)) {
        PyObject *args = PyTuple_Pack(4, list_in, PyLong_FromLong(ELEMENT_SIZE_BITS(arr_in)), list_out, PyLong_FromLong(ELEMENT_SIZE_BITS(arr_out)));
        PyObject *ret_obj = PyObject_CallObject(callable, args);
        recursive_set(iters_out, dim_out - 1, 0, arr_out, ret_obj);
        Py_DecRef(args);
    }

    Py_DecRef(list_in);
    
    free(iters_in);
    free(iters_out);

    /* To view the start-end indices of inner layer, using the following instructions: */
    // int inner_low = svLow(arr_in, 0);
    // int inner_high = svHigh(arr_in, 0);
}
