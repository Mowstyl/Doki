#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <omp.h>
#include "platform.h"
#include "qstate.h"
#include "qgate.h"
#include "qops.h"


static PyObject *DokiError;

void
doki_registry_destroy (PyObject *capsule);

void
doki_gate_destroy (PyObject *capsule);

void
doki_funmatrix_destroy (PyObject *capsule);

static PyObject *
doki_registry_new (PyObject *self, PyObject *args);

static PyObject *
doki_registry_clone (PyObject *self, PyObject *args);

static PyObject *
doki_registry_del (PyObject *self, PyObject *args);

static PyObject *
doki_gate_new (PyObject *self, PyObject *args);

static PyObject *
doki_gate_get (PyObject *self, PyObject *args);

static PyObject *
doki_registry_get (PyObject *self, PyObject *args);

static PyObject *
doki_registry_apply (PyObject *self, PyObject *args);

static PyObject *
doki_registry_join (PyObject *self, PyObject *args);

static PyObject *
doki_registry_measure (PyObject *self, PyObject *args);

static PyObject *
doki_registry_prob (PyObject *self, PyObject *args);

static PyObject *
doki_registry_density (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_get (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_add (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_sub (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_scalar_mul (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_scalar_div (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_matmul (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_ewmul (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_kron (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_transpose (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_dagger (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_shape (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_partialtrace (PyObject *self, PyObject *args);

static PyObject *
doki_funmatrix_trace (PyObject *self, PyObject *args);

static PyMethodDef DokiMethods[] = {
    {"gate_new", doki_gate_new, METH_VARARGS, "Create new gate"},
    {"gate_get", doki_gate_get, METH_VARARGS, "Get matrix associated to gate"},
    {"registry_new", doki_registry_new, METH_VARARGS, "Create new registry"},
    {"registry_clone", doki_registry_clone, METH_VARARGS, "Clone a registry"},
    {"registry_del", doki_registry_del, METH_VARARGS, "Destroy a registry"},
    {"registry_get", doki_registry_get, METH_VARARGS, "Get value from registry"},
    {"registry_apply", doki_registry_apply, METH_VARARGS, "Apply a gate"},
    {"registry_join", doki_registry_join, METH_VARARGS, "Merges two registries"},
    {"registry_measure", doki_registry_measure, METH_VARARGS, "Measures and collapses specified qubits"},
    {"registry_prob", doki_registry_prob, METH_VARARGS, "Get the chances of obtaining 1 when measuring a certain qubit"},
    {"registry_density", doki_registry_density, METH_VARARGS, "Get the density matrix"},
    {"funmatrix_get", doki_funmatrix_get, METH_VARARGS, "Get a value from a functional matrix"},
    {"funmatrix_add", doki_funmatrix_add, METH_VARARGS, "Get the addition of two functional matrices"},
    {"funmatrix_sub", doki_funmatrix_sub, METH_VARARGS, "Get the substraction of two functional matrices"},
    {"funmatrix_scalar_mul", doki_funmatrix_scalar_mul, METH_VARARGS, "Get the product of a scalar and a functional matrix"},
    {"funmatrix_scalar_div", doki_funmatrix_scalar_div, METH_VARARGS, "Get the division of a functional matrix by a scalar"},
    {"funmatrix_matmul", doki_funmatrix_matmul, METH_VARARGS, "Get the matrix product of two functional matrices"},
    {"funmatrix_ewmul", doki_funmatrix_ewmul, METH_VARARGS, "Get the entity-wise multiplication of two functional matrices"},
    {"funmatrix_kron", doki_funmatrix_kron, METH_VARARGS, "Get the Kronecker product of two functional matrices"},
    {"funmatrix_transpose", doki_funmatrix_transpose, METH_VARARGS, "Get the transpose of a functional matrix"},
    {"funmatrix_dagger", doki_funmatrix_dagger, METH_VARARGS, "Get the conjugate-transpose of a functional matrix"},
    {"funmatrix_shape", doki_funmatrix_shape, METH_VARARGS, "Get a tuple with the shape of the matrix"},
    {"funmatrix_partialtrace", doki_funmatrix_partialtrace, METH_VARARGS, "Get the partial trace of a functional matrix"},
    {"funmatrix_trace", doki_funmatrix_trace, METH_VARARGS, "Get the trace of a functional matrix"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef dokimodule = {
    PyModuleDef_HEAD_INIT,
    "doki",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    DokiMethods
};

PyMODINIT_FUNC
PyInit_doki(void)
{
    PyObject *m;

    m = PyModule_Create(&dokimodule);
    if (m == NULL)
        return NULL;

    DokiError = PyErr_NewException("qsimov.doki.error", NULL, NULL);
    Py_XINCREF(DokiError);
    if (PyModule_AddObject(m, "error", DokiError) < 0) {
        Py_XDECREF(DokiError);
        Py_CLEAR(DokiError);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

int
main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("doki", PyInit_doki) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    PyObject *pmodule = PyImport_ImportModule("doki");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'spam'\n");
    }
    PyMem_RawFree(program);
    return 0;
}

void
doki_registry_destroy (PyObject *capsule)
{
    struct state_vector *state;
    void *raw_state;
    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");

    if (raw_state != NULL) {
        state = (struct state_vector*) raw_state;
        state_clear(state);
        free(state);
    }
}

void
doki_gate_destroy (PyObject *capsule)
{
    struct qgate *gate;
    void *raw_gate;
    NATURAL_TYPE i;

    raw_gate = PyCapsule_GetPointer(capsule, "qsimov.doki.gate");

    if (raw_gate != NULL) {
        gate = (struct qgate*) raw_gate;

        for (i = 0; i < gate->size; i++) {
            free(gate->matrix[i]);
        }
        free(gate->matrix);
        free(gate);
    }
}

void
doki_funmatrix_destroy (PyObject *capsule)
{
    FunctionalMatrix *matrix;
    void *raw_matrix;

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix != NULL) {
        matrix = (FunctionalMatrix*) raw_matrix;
        free(matrix);
    }
}

static PyObject *
doki_registry_new (PyObject *self, PyObject *args)
{
    unsigned int num_qubits;
    unsigned char result;
    struct state_vector *state;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Ip", &num_qubits, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_new(num_qubits, verbose)");
        return NULL;
    }
    if (num_qubits == 0) {
        PyErr_SetString(DokiError, "num_qubits can't be zero");
        return NULL;
    }

    state = MALLOC_TYPE(1, struct state_vector);
    if (state == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate state structure");
        return NULL;
    }
    result = state_init(state, num_qubits, 1);
    if (result == 1) {
        PyErr_SetString(DokiError, "Failed to allocate state vector");
        return NULL;
    }
    else if (result == 2) {
        PyErr_SetString(DokiError, "Failed to allocate state chunk");
        return NULL;
    }
    else if (result == 3) {
        PyErr_SetString(DokiError, "Number of qubits exceeds maximum");
        return NULL;
    }
    else if (result != 0) {
        PyErr_SetString(DokiError, "Unknown error when creating state");
        return NULL;
    }
    return PyCapsule_New((void*) state, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
}

static PyObject *
doki_registry_clone (PyObject *self, PyObject *args)
{
    PyObject *source_capsule;
    unsigned char result;
    void *raw_source;
    struct state_vector *source, *dest;
    int num_threads, debug_enabled;

    if (!PyArg_ParseTuple(args, "Oip", &source_capsule,
                          &num_threads, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_clone(registry, num_threads, verbose)");
        return NULL;
    }

    if (num_threads <= 0 && num_threads != -1) {
        PyErr_SetString(DokiError, "num_threads must be at least 1 (or -1 to let OpenMP choose)");
        return NULL;
    }

    raw_source = PyCapsule_GetPointer(source_capsule, "qsimov.doki.state_vector");
    if (raw_source == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to source registry");
        return NULL;
    }
    source = (struct state_vector*) raw_source;

    dest = MALLOC_TYPE(1, struct state_vector);
    if (dest == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate new state structure");
        return NULL;
    }

    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }

    result = state_clone(dest, source);
    if (result == 1) {
        PyErr_SetString(DokiError, "Failed to allocate state vector");
        return NULL;
    }
    else if (result == 2) {
        PyErr_SetString(DokiError, "Failed to allocate state chunk");
        return NULL;
    }
    else if (result != 0) {
        PyErr_SetString(DokiError, "Unknown error when cloning state");
        return NULL;
    }
    return PyCapsule_New((void*) dest, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
}

static PyObject *
doki_registry_del (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_del(registry, verbose)");
        return NULL;
    }

    doki_registry_destroy(capsule);
    PyCapsule_SetDestructor(capsule, NULL);

    Py_RETURN_NONE;
}

static PyObject *
doki_gate_new (PyObject *self, PyObject *args)
{
    PyObject *list, *row, *raw_val;
    unsigned int num_qubits;
    NATURAL_TYPE i, j, k;
    COMPLEX_TYPE val;
    struct qgate *gate;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "IOp", &num_qubits, &list, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: gate_new(num_qubits, gate, verbose)");
        return NULL;
    }
    if (num_qubits == 0) {
        PyErr_SetString(DokiError, "num_qubits can't be zero");
        return NULL;
    }
    if (!PyList_Check(list)) {
        PyErr_SetString(DokiError, "gate must be a list of lists (matrix)");
        return NULL;
    }

    gate = MALLOC_TYPE(1, struct qgate);
    if (gate == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate qgate");
        return NULL;
    }

    gate->num_qubits = num_qubits;
    gate->size = NATURAL_ONE << num_qubits;
    if ((NATURAL_TYPE) PyList_Size(list) != gate->size) {
        PyErr_SetString(DokiError, "Wrong matrix size for specified number of qubits");
        free(gate);
        return NULL;
    }

    gate->matrix = MALLOC_TYPE(gate->size, COMPLEX_TYPE*);
    if (gate->matrix == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate qgate matrix");
        free(gate);
        return NULL;
    }

    for (i = 0; i < gate->size; i++) {
        row = PyList_GetItem(list, i);
        if (!PyList_Check(row) || (NATURAL_TYPE) PyList_Size(row) != gate->size) {
            PyErr_SetString(DokiError, "rows must be lists of size 2^num_qubits");
            for (k = 0; k < i; k++) {
                free(gate->matrix[k]);
            }
            free(gate->matrix);
            free(gate);
            return NULL;
        }
        gate->matrix[i] = MALLOC_TYPE(gate->size, COMPLEX_TYPE);
        for (j = 0; j < gate->size; j++) {
            raw_val = PyList_GetItem(row, j);
            if (PyComplex_Check(raw_val)) {
                val = complex_init(PyComplex_RealAsDouble(raw_val), PyComplex_ImagAsDouble(raw_val));
            }
            else if(PyFloat_Check(raw_val)) {
                val = complex_init(PyFloat_AsDouble(raw_val), 0.0);
            }
            else if(PyLong_Check(raw_val)) {
                val = complex_init((double) PyLong_AsLong(raw_val), 0.0);
            }
            else {
                PyErr_SetString(DokiError, "matrix elements must be complex numbers");
                for (k = 0; k <= i; k++) {
                    free(gate->matrix[k]);
                }
                free(gate->matrix);
                free(gate);
                return NULL;
            }
            gate->matrix[i][j] = val;
        }
    }

    return PyCapsule_New((void*) gate, "qsimov.doki.gate", &doki_gate_destroy);
}


static PyObject *
doki_gate_get (PyObject *self, PyObject *args)
{
    PyObject *capsule, *result, *aux;
    COMPLEX_TYPE val;
    NATURAL_TYPE i, j;
    void *raw_gate;
    struct qgate *gate;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: gate_get(gate, verbose)");
        return NULL;
    }

    raw_gate = PyCapsule_GetPointer(capsule, "qsimov.doki.gate");
    if (raw_gate == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to gate");
        return NULL;
    }
    gate = (struct qgate*) raw_gate;
    result = PyList_New(gate->size);
    for (i = 0; i < gate->size; i++) {
        aux = PyList_New(gate->size);
        for (j = 0; j < gate->size; j++) {
            val = gate->matrix[i][j];
            PyList_SET_ITEM(aux, j, PyComplex_FromDoubles(RE(val), IM(val)));
        }
        PyList_SET_ITEM(result, i, aux);
    }

    return result;
}

static PyObject *
doki_registry_get (PyObject *self, PyObject *args)
{
    PyObject *capsule, *result;
    void *raw_state;
    struct state_vector *state;
    NATURAL_TYPE id;
    COMPLEX_TYPE val, aux;
    REAL_TYPE phase;
    int canonical, debug_enabled;

    if (!PyArg_ParseTuple(args, "OKpp", &capsule, &id, &canonical, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_get(registry, id, canonical, verbose)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    state = (struct state_vector*) raw_state;
    val = state_get(state, id);
    if (debug_enabled) {
        printf("[DEBUG] raw = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(state->vector[id/COMPLEX_ARRAY_SIZE][id%COMPLEX_ARRAY_SIZE]));
        printf("[DEBUG] normconst = %lf\n", state->norm_const);
        printf("[DEBUG] res = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(val));
    }
    if (canonical) {
        phase = get_global_phase(state);
        if (debug_enabled) {
            printf("[DEBUG] phase = " REAL_STRING_FORMAT "\n", phase);
        }
        aux = complex_init(COS(phase), -SIN(phase));
        val = complex_mult(val, aux);
    }
    result = PyComplex_FromDoubles(RE(val), IM(val));

    return result;
}

static PyObject *
doki_registry_apply (PyObject *self, PyObject *args)
{
    PyObject *raw_val, *state_capsule, *gate_capsule,
             *target_list, *control_set, *acontrol_set, *aux;
    void *raw_state, *raw_gate;
    struct state_vector *state, *new_state;
    struct qgate *gate;
    unsigned char exit_code;
    unsigned int num_targets, num_controls, num_anticontrols, i;
    unsigned int *targets, *controls, *anticontrols;
    int num_threads, debug_enabled;

    if (!PyArg_ParseTuple(args, "OOOOOip", &state_capsule, &gate_capsule,
                          &target_list, &control_set, &acontrol_set, &num_threads, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_apply(registry, gate, target_list, control_set, anticontrol_set, num_threads, verbose)");
        return NULL;
    }

    if (num_threads <= 0 && num_threads != -1) {
        PyErr_SetString(DokiError, "num_threads must be at least 1 (or -1 to let OpenMP choose)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(state_capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    state = (struct state_vector*) raw_state;

    raw_gate = PyCapsule_GetPointer(gate_capsule, "qsimov.doki.gate");
    if (raw_gate == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to gate");
        return NULL;
    }
    gate = (struct qgate*) raw_gate;

    if (!PyList_Check(target_list)) {
        PyErr_SetString(DokiError, "target_list must be a list");
        return NULL;
    }

    num_targets = (unsigned int) PyList_Size(target_list);
    if (num_targets != gate->num_qubits) {
        PyErr_SetString(DokiError, "Wrong number of targets specified for that gate");
        return NULL;
    }

    num_controls = 0;
    if (PySet_Check(control_set)) {
        num_controls = (unsigned int) PySet_Size(control_set);
    }
    else if (control_set != Py_None) {
        PyErr_SetString(DokiError, "control_set must be a set or None");
        return NULL;
    }

    num_anticontrols = 0;
    if (PySet_Check(acontrol_set)) {
        num_anticontrols = (unsigned int) PySet_Size(acontrol_set);
    }
    else if (acontrol_set != Py_None) {
        PyErr_SetString(DokiError, "anticontrol_set must be a set or None");
        return NULL;
    }

    targets = MALLOC_TYPE(num_targets, unsigned int);
    if (targets == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate target array");
        return NULL;
    }
    controls = NULL;
    if (num_controls > 0) {
        controls = MALLOC_TYPE(num_controls, unsigned int);
        if (controls == NULL) {
            PyErr_SetString(DokiError, "Failed to allocate control array");
            return NULL;
        }
    }
    anticontrols = NULL;
    if (num_anticontrols > 0) {
        anticontrols = MALLOC_TYPE(num_anticontrols, unsigned int);
        if (anticontrols == NULL) {
            PyErr_SetString(DokiError, "Failed to allocate anticontrol array");
            return NULL;
        }
    }

    if (num_controls > 0) {
        aux = PySet_New(control_set);
        for (i = 0; i < num_controls; i++) {
            raw_val = PySet_Pop(aux);
            if(!PyLong_Check(raw_val)) {
                PyErr_SetString(DokiError, "control_set must be a set qubit ids (unsigned integers)");
                return NULL;
            }
            controls[i] = PyLong_AsLong(raw_val);
            if (controls[i] >= state->num_qubits) {
                PyErr_SetString(DokiError, "Control qubit out of range");
                return NULL;
            }
        }
    }

    if (num_anticontrols > 0) {
        aux = PySet_New(acontrol_set);
        for (i = 0; i < num_anticontrols; i++) {
            raw_val = PySet_Pop(aux);
            if(!PyLong_Check(raw_val)) {
                PyErr_SetString(DokiError, "anticontrol_set must be a set qubit ids (unsigned integers)");
                return NULL;
            }
            if (PySet_Contains(control_set, raw_val)) {
                PyErr_SetString(DokiError, "A control cannot also be an anticontrol");
                return NULL;
            }
            anticontrols[i] = PyLong_AsLong(raw_val);
            if (anticontrols[i] >= state->num_qubits) {
                PyErr_SetString(DokiError, "Anticontrol qubit out of range");
                return NULL;
            }
        }
    }

    for (i = 0; i < num_targets; i++) {
        raw_val = PyList_GetItem(target_list, i);
        if(!PyLong_Check(raw_val)) {
            PyErr_SetString(DokiError, "target_list must be a list of qubit ids (unsigned integers)");
            return NULL;
        }
        if ((num_controls > 0 && PySet_Contains(control_set, raw_val)) || (num_anticontrols > 0 && PySet_Contains(acontrol_set, raw_val))) {
            PyErr_SetString(DokiError, "A target cannot also be a control or an anticontrol");
            return NULL;
        }
        targets[i] = PyLong_AsLong(raw_val);
        if (targets[i] >= state->num_qubits) {
            PyErr_SetString(DokiError, "Target qubit out of range");
            return NULL;
        }
    }

    new_state = MALLOC_TYPE(1, struct state_vector);
    if (new_state == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate new state structure");
        return NULL;
    }
    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }
    // printf("[DEBUG] nums: %u, %u, %u\n", num_targets, num_controls, num_anticontrols);
    exit_code = apply_gate(state, gate, targets, num_targets, controls,
                           num_controls, anticontrols, num_anticontrols,
                           new_state);

    if (exit_code == 1) {
        PyErr_SetString(DokiError, "Failed to initialize new state chunk");
    }
    else if (exit_code == 2) {
        PyErr_SetString(DokiError, "Failed to allocate new state chunk");
    }
    else if (exit_code == 3) {
        PyErr_SetString(DokiError, "[BUG] THIS SHOULD NOT HAPPEN. Failed to set first value to 1");
    }
    else if (exit_code == 4) {
        PyErr_SetString(DokiError, "Failed to allocate new state vector structure");
    }
    else if (exit_code == 5) {
        PyErr_SetString(DokiError, "Failed to apply gate");
    }
    else if (exit_code == 11) {
        PyErr_SetString(DokiError, "Failed to allocate not_copy structure");
    }
    else if (exit_code != 0) {
        PyErr_SetString(DokiError, "Unknown error when applying gate");
    }

    if (exit_code > 0) {
        free(targets);
        if (num_controls > 0) {
            free(controls);
        }
        if (num_anticontrols > 0) {
            free(anticontrols);
        }
        return NULL;
    }

    return PyCapsule_New((void*) new_state, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
}

static PyObject *
doki_registry_join (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_state1, *raw_state2;
    struct state_vector *state1, *state2, *result;
    unsigned char exit_code;
    int num_threads, debug_enabled;

    if (!PyArg_ParseTuple(args, "OOip", &capsule1, &capsule2, &num_threads, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_join(most_registry, least_registry, num_threads, verbose)");
        return NULL;
    }

    if (num_threads <= 0 && num_threads != -1) {
        PyErr_SetString(DokiError, "num_threads must be at least 1 (or -1 to let OpenMP choose)");
        return NULL;
    }

    raw_state1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.state_vector");
    if (raw_state1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry1");
        return NULL;
    }

    raw_state2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.state_vector");
    if (raw_state2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry2");
        return NULL;
    }
    state1 = (struct state_vector*) raw_state1;
    state2 = (struct state_vector*) raw_state2;
    result = MALLOC_TYPE(1, struct state_vector);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate new state structure");
        return NULL;
    }
    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }
    exit_code = join(result, state1, state2);
    if (exit_code != 0) {
        switch (exit_code) {
            case 1:
                PyErr_SetString(DokiError, "Failed to initialize new state chunk");
                break;
            case 2:
                PyErr_SetString(DokiError, "Failed to allocate new state chunk");
                break;
            case 3:
                PyErr_SetString(DokiError, "[BUG] THIS SHOULD NOT HAPPEN. Failed to set first value to 1");
                break;
            case 4:
                PyErr_SetString(DokiError, "Failed to allocate new state vector structure");
                break;
            case 5:
                PyErr_SetString(DokiError, "Failed to get/set a value");
                break;
            default:
                PyErr_SetString(DokiError, "Unknown error when joining states");
        }
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
}

static PyObject *
doki_registry_measure (PyObject *self, PyObject *args)
{
    PyObject *capsule, *py_measured_val, *result, *new_capsule, *roll_list;
    Py_ssize_t roll_id;
    void *raw_state;
    struct state_vector *state, *new_state, *aux;
    NATURAL_TYPE mask;
    REAL_TYPE roll;
    unsigned int i, curr_id, initial_num_qubits, measured_qty;
    _Bool measure_id, measured_val;
    unsigned char exit_code;
    int debug_enabled, num_threads;

    if (!PyArg_ParseTuple(args, "OKOip", &capsule, &mask, &roll_list, &num_threads, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_measure(registry, mask, roll_list, num_threads, verbose)");
        return NULL;
    }

    if (num_threads <= 0 && num_threads != -1) {
        PyErr_SetString(DokiError, "num_threads must be at least 1 (or -1 to let OpenMP choose)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    if (!PyList_Check(roll_list)) {
        PyErr_SetString(DokiError, "roll_list must be a list of real numbers in [0, 1)!");
        return NULL;
    }
    state = (struct state_vector*) raw_state;
    initial_num_qubits = state->num_qubits;
    result = PyList_New(initial_num_qubits);

    new_state = MALLOC_TYPE(1, struct state_vector);
    if (new_state == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate new state structure");
        return NULL;
    }

    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }
    exit_code = state_clone(new_state, state);
    if (exit_code == 1) {
        PyErr_SetString(DokiError, "Failed to allocate state vector");
        return NULL;
    }
    else if (exit_code == 2) {
        PyErr_SetString(DokiError, "Failed to allocate state chunk");
        return NULL;
    }
    else if (exit_code == 3) {
        if (debug_enabled) {
            printf("[DEBUG] %u", state->num_qubits);
        }
        PyErr_SetString(DokiError, "Wrong number of qubits");
        return NULL;
    }
    else if (exit_code != 0) {
        PyErr_SetString(DokiError, "Unknown error when cloning state");
        return NULL;
    }

    measured_qty = 0;
    roll_id = 0;
    aux = NULL;
    for (i = 0; i < initial_num_qubits; i++) {
        curr_id = initial_num_qubits - i - 1;
        measure_id = mask & (NATURAL_ONE << curr_id);
        py_measured_val = Py_None;
        if (measure_id) {
            if (new_state == NULL || new_state->num_qubits == 0) {
                if (new_state != NULL) {
                    state_clear(new_state);
                    free(new_state);
                }
                PyErr_SetString(DokiError, "Could not measure non_existant qubits");
                return NULL;
            }
            roll = PyFloat_AsDouble(PyList_GetItem(roll_list, roll_id));
            if (roll < 0 || roll >= 1) {
                state_clear(new_state);
                free(new_state);
                PyErr_SetString(DokiError, "roll not in interval [0, 1)!");
                return NULL;
            }
            roll_id++;
            aux = MALLOC_TYPE(1, struct state_vector);
            if (aux == NULL) {
                state_clear(new_state);
                free(new_state);
                PyErr_SetString(DokiError, "Failed to allocate aux state structure");
                return NULL;
            }
            exit_code = measure(new_state, &measured_val, curr_id, aux, roll);
            if (exit_code != 0) {
                state_clear(aux);
                free(aux);
                aux = NULL;
                break;
            }
            if (aux->num_qubits > 0 && aux->norm_const == 0.0) {
                state_clear(aux);
                free(aux);
                state_clear(new_state);
                free(new_state);
                PyErr_SetString(DokiError, "New normalization constant is 0. Please report this error with the steps to reproduce it.");
                return NULL;
            }
            measured_qty++;
            py_measured_val = measured_val ? Py_True : Py_False;
            state_clear(new_state);
            free(new_state);
            new_state = aux;
            aux = NULL;
        }
        PyList_SET_ITEM(result, i, py_measured_val);
    }
    if (exit_code != 0) {
        if (new_state != NULL) {
            state_clear(new_state);
            free(new_state);
        }
        switch (exit_code) {
            case 1:
                PyErr_SetString(DokiError, "Failed to allocate state vector");
                break;
            case 2:
                PyErr_SetString(DokiError, "Failed to allocate state chunk");
                break;
            default:
                PyErr_SetString(DokiError, "Unknown error while collapsing state");
        }
        return NULL;
    }

    if (state->num_qubits - measured_qty > 0) {
        new_capsule = PyCapsule_New((void*) new_state, "qsimov.doki.state_vector",
                                    &doki_registry_destroy);
    }
    else {
        if (new_state != NULL) {
            state_clear(new_state);
            free(new_state);
        }
        new_capsule = Py_None;
    }
    return PyTuple_Pack(2, new_capsule, result);
}

static PyObject *
doki_registry_prob (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_state;
    struct state_vector *state;
    unsigned int id;
    int debug_enabled, num_threads;

    if (!PyArg_ParseTuple(args, "OIip", &capsule, &id, &num_threads, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: registry_prob(registry, qubit_id, num_threads, verbose)");
        return NULL;
    }

    if (num_threads <= 0 && num_threads != -1) {
        PyErr_SetString(DokiError, "num_threads must be at least 1 (or -1 to let OpenMP choose)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    state = (struct state_vector*) raw_state;

    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }
    return PyFloat_FromDouble(probability(state, id));
}

static PyObject *
doki_registry_density (PyObject *self, PyObject *args)
{
    PyObject *state_capsule;
    void *raw_state;
    FunctionalMatrix *densityMatrix;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &state_capsule, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: registry_density(state, verbose)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(state_capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }

    densityMatrix = densityMat((struct state_vector*) raw_state);
    if (densityMatrix == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate density matrix");
        return NULL;
    }

    return PyCapsule_New((void*) densityMatrix, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_get (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_matrix;
    FunctionalMatrix *matrix;
    NATURAL_TYPE i, j;
    COMPLEX_TYPE val;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OKKp", &capsule, &i, &j, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_get(funmatrix, i, j, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    if (i < 0 || j < 0 || i >= matrix->r || j >= matrix->c) {
        PyErr_SetString(DokiError, "Out of bounds");
        return NULL;
    }

    if (!getitem(matrix, i, j, &val)) {
        PyErr_SetString(DokiError, "Error getting element");
        return NULL;
    }
	// printf("absol = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(val));

    if (isnan(RE(val)) || isnan(IM(val))) {
        PyErr_SetString(DokiError, "Error calculating element");
        return NULL;
    }
	// printf("fabsol = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(val));

    return PyComplex_FromDoubles(RE(val), IM(val));
}

static PyObject *
doki_funmatrix_add (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_matrix1, *raw_matrix2;
    FunctionalMatrix *matrix1, *matrix2, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule1, &capsule2, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_add(funmatrix1, funmatrix2, verbose)");
        return NULL;
    }

    raw_matrix1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.funmatrix");
    if (raw_matrix1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 1");
        return NULL;
    }
    matrix1 = (FunctionalMatrix*) raw_matrix1;

    raw_matrix2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.funmatrix");
    if (raw_matrix2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 2");
        return NULL;
    }
    matrix2 = (FunctionalMatrix*) raw_matrix2;

    result = madd(matrix1, matrix2);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_sub (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_matrix1, *raw_matrix2;
    FunctionalMatrix *matrix1, *matrix2, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule1, &capsule2, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_sub(funmatrix1, funmatrix2, verbose)");
        return NULL;
    }

    raw_matrix1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.funmatrix");
    if (raw_matrix1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 1");
        return NULL;
    }
    matrix1 = (FunctionalMatrix*) raw_matrix1;

    raw_matrix2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.funmatrix");
    if (raw_matrix2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 2");
        return NULL;
    }
    matrix2 = (FunctionalMatrix*) raw_matrix2;

    result = msub(matrix1, matrix2);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_scalar_mul (PyObject *self, PyObject *args)
{
    PyObject *capsule, *raw_scalar;
    void *raw_matrix;
    FunctionalMatrix *matrix, *result;
    COMPLEX_TYPE scalar;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule, &raw_scalar, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_mul(funmatrix, scalar, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    if (PyComplex_Check(raw_scalar)) {
        scalar = complex_init(PyComplex_RealAsDouble(raw_scalar), PyComplex_ImagAsDouble(raw_scalar));
    }
    else if(PyFloat_Check(raw_scalar)) {
        scalar = complex_init(PyFloat_AsDouble(raw_scalar), 0.0);
    }
    else if(PyLong_Check(raw_scalar)) {
        scalar = complex_init((double) PyLong_AsLong(raw_scalar), 0.0);
    }
    else {
        PyErr_SetString(DokiError, "scalar is not a number");
        return NULL;
    }

    result = mprod(scalar, matrix);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_scalar_div (PyObject *self, PyObject *args)
{
    PyObject *capsule, *raw_scalar;
    void *raw_matrix;
    FunctionalMatrix *matrix, *result;
    COMPLEX_TYPE scalar;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule, &raw_scalar, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_div(funmatrix, scalar, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    if (PyComplex_Check(raw_scalar)) {
        scalar = complex_init(PyComplex_RealAsDouble(raw_scalar), PyComplex_ImagAsDouble(raw_scalar));
    }
    else if(PyFloat_Check(raw_scalar)) {
        scalar = complex_init(PyFloat_AsDouble(raw_scalar), 0.0);
    }
    else if(PyLong_Check(raw_scalar)) {
        scalar = complex_init((double) PyLong_AsLong(raw_scalar), 0.0);
    }
    else {
        PyErr_SetString(DokiError, "scalar is not a number");
        return NULL;
    }

    result = mdiv(scalar, matrix);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_matmul (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_matrix1, *raw_matrix2;
    FunctionalMatrix *matrix1, *matrix2, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule1, &capsule2, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_matmul(funmatrix1, funmatrix2, verbose)");
        return NULL;
    }

    raw_matrix1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.funmatrix");
    if (raw_matrix1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 1");
        return NULL;
    }
    matrix1 = (FunctionalMatrix*) raw_matrix1;

    raw_matrix2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.funmatrix");
    if (raw_matrix2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 2");
        return NULL;
    }
    matrix2 = (FunctionalMatrix*) raw_matrix2;

    result = matmul(matrix1, matrix2);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_ewmul (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_matrix1, *raw_matrix2;
    FunctionalMatrix *matrix1, *matrix2, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule1, &capsule2, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_ewmul(funmatrix1, funmatrix2, verbose)");
        return NULL;
    }

    raw_matrix1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.funmatrix");
    if (raw_matrix1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 1");
        return NULL;
    }
    matrix1 = (FunctionalMatrix*) raw_matrix1;

    raw_matrix2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.funmatrix");
    if (raw_matrix2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 2");
        return NULL;
    }
    matrix2 = (FunctionalMatrix*) raw_matrix2;

    result = ewmul(matrix1, matrix2);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_kron (PyObject *self, PyObject *args)
{
    PyObject *capsule1, *capsule2;
    void *raw_matrix1, *raw_matrix2;
    FunctionalMatrix *matrix1, *matrix2, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OOp", &capsule1, &capsule2, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_kron(funmatrix1, funmatrix2, verbose)");
        return NULL;
    }

    raw_matrix1 = PyCapsule_GetPointer(capsule1, "qsimov.doki.funmatrix");
    if (raw_matrix1 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 1");
        return NULL;
    }
    matrix1 = (FunctionalMatrix*) raw_matrix1;

    raw_matrix2 = PyCapsule_GetPointer(capsule2, "qsimov.doki.funmatrix");
    if (raw_matrix2 == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix 2");
        return NULL;
    }
    matrix2 = (FunctionalMatrix*) raw_matrix2;

    result = kron(matrix1, matrix2);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_transpose (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_matrix;
    FunctionalMatrix *matrix, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_transpose(funmatrix, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    result = transpose(matrix);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_dagger (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_matrix;
    FunctionalMatrix *matrix, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_dagger(funmatrix, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    result = dagger(matrix);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_shape (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_matrix;
    FunctionalMatrix *matrix;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled))
    {
        PyErr_SetString(DokiError, "Syntax: funmatrix_shape(funmatrix, verbose)");
        return NULL;
    }

    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;

    return PyTuple_Pack(2, PyLong_FromUnsignedLongLong(rows(matrix)),
                           PyLong_FromUnsignedLongLong(columns(matrix)));
}

static PyObject *
doki_funmatrix_partialtrace (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    unsigned int id;
    void *raw_matrix;
    FunctionalMatrix *matrix, *result;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "OIp", &capsule, &id, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: funmatrix_partialtrace(funmatrix, id, verbose)");
        return NULL;
    }
    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;
    result = partial_trace(matrix, id);
    if (result == NULL) {
        PyErr_SetString(DokiError, "Failed to allocate result");
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.funmatrix", &doki_funmatrix_destroy);
}

static PyObject *
doki_funmatrix_trace (PyObject *self, PyObject *args)
{
    PyObject *capsule;
    void *raw_matrix;
    FunctionalMatrix *matrix;
    COMPLEX_TYPE result, aux;
    NATURAL_TYPE i, min_shape;
    int debug_enabled;

    if (!PyArg_ParseTuple(args, "Op", &capsule, &debug_enabled)) {
        PyErr_SetString(DokiError, "Syntax: funmatrix_trace(funmatrix, verbose)");
        return NULL;
    }
    raw_matrix = PyCapsule_GetPointer(capsule, "qsimov.doki.funmatrix");
    if (raw_matrix == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to matrix");
        return NULL;
    }
    matrix = (FunctionalMatrix*) raw_matrix;
    result = COMPLEX_ZERO;
    min_shape = matrix->r <= matrix->c ? matrix->r : matrix->c;

    aux = COMPLEX_ZERO;
    for (i = 0; i < min_shape; i++) {
        if (!getitem(matrix, i, i, &aux)) {
            PyErr_SetString(DokiError, "Failed to get matrix element");
            return NULL;
        }
        result = complex_sum(result, aux);
    }

    return PyComplex_FromDoubles(RE(result), IM(result));
}
