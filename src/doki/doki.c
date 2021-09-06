#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "platform.h"
#include "qstate.h"
#include "qgate.h"
#include "qops.h"


static PyObject *DokiError;

void
doki_registry_destroy (PyObject *capsule);

void
doki_gate_destroy (PyObject *capsule);

static PyObject *
doki_seed_set (PyObject *self, PyObject *args);

static PyObject *
doki_registry_new (PyObject *self, PyObject *args);

static PyObject *
doki_registry_gate (PyObject *self, PyObject *args);

static PyObject *
doki_registry_get (PyObject *self, PyObject *args);

static PyObject *
doki_registry_apply (PyObject *self, PyObject *args);

static PyObject *
doki_registry_join (PyObject *self, PyObject *args);

static PyObject *
doki_registry_measure (PyObject *self, PyObject *args);

static PyMethodDef DokiMethods[] = {
    {"seed", doki_seed_set, METH_VARARGS, "Set seed for random operations"},
    {"new", doki_registry_new, METH_VARARGS, "Create new registry"},
    {"gate", doki_registry_gate, METH_VARARGS, "Create new gate"},
    {"get", doki_registry_get, METH_VARARGS, "Get value from registry"},
    {"apply", doki_registry_apply, METH_VARARGS, "Apply a gate"},
    {"join", doki_registry_join, METH_VARARGS, "Merges two registries"},
    {"measure", doki_registry_measure, METH_VARARGS, "Measures and collapses specified qubits"},
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
    state = (struct state_vector*) raw_state;
    state_clear(state);
    free(state);
}

void
doki_gate_destroy (PyObject *capsule)
{
    struct qgate *gate;
    void *raw_gate;
    NATURAL_TYPE i;

    raw_gate = PyCapsule_GetPointer(capsule, "qsimov.doki.gate");
    gate = (struct qgate*) raw_gate;

    for (i = 0; i < gate->size; i++) {
        free(gate->matrix[i]);
    }
    free(gate->matrix);
    free(gate);
}

static PyObject *
doki_seed_set (PyObject *self, PyObject *args)
{
    long long seed, random;
    if (!PyArg_ParseTuple(args, "L", &seed))
    {
        PyErr_SetString(DokiError, "Syntax: seed(integer number)");
        return NULL;
    }
    // fprintf(stdout, "Seed: %lld\n", seed);
    srand(seed);
}

static PyObject *
doki_registry_new (PyObject *self, PyObject *args)
{
    unsigned int num_qubits;
    unsigned char result;
    struct state_vector *state;

    if (!PyArg_ParseTuple(args, "I", &num_qubits))
    {
        PyErr_SetString(DokiError, "Syntax: new(num_qubits)");
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
        PyErr_SetString(DokiError, "Failed to initialize state chunk");
        return NULL;
    }
    else if (result == 2) {
        PyErr_SetString(DokiError, "Failed to allocate state chunk");
        return NULL;
    }
    else if (result == 3) {
        PyErr_SetString(DokiError, "Failed to set first value to 1");
        return NULL;
    }
    else if (result == 4) {
        PyErr_SetString(DokiError, "Failed to allocate state vector structure");
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
doki_registry_gate (PyObject *self, PyObject *args)
{
    PyObject *list, *row, *raw_val;
    unsigned int num_qubits;
    NATURAL_TYPE i, j, k;
    COMPLEX_TYPE val;
    struct qgate *gate;

    if (!PyArg_ParseTuple(args, "IO", &num_qubits, &list))
    {
        PyErr_SetString(DokiError, "Syntax: gate(num_qubits, gate)");
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
    if (PyList_Size(list) != gate->size) {
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
        if (!PyList_Check(row) || PyList_Size(row) != gate->size) {
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
doki_registry_get (PyObject *self, PyObject *args)
{
    PyObject *capsule, *result;
    void *raw_state;
    struct state_vector *state;
    NATURAL_TYPE id;
    COMPLEX_TYPE val;
    unsigned char exit_code;

    if (!PyArg_ParseTuple(args, "OK", &capsule, &id)) {
        PyErr_SetString(DokiError, "Syntax: get(registry, id)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    state = (struct state_vector*) raw_state;
    exit_code = state_get(state, id, &val);
    if (exit_code == 1) {
        PyErr_SetString(DokiError, "Not here");
        return NULL;
    }
    else if (exit_code == 2) {
        PyErr_SetString(DokiError, "Out of bounds");
        return NULL;
    }
    result = PyComplex_FromDoubles(creal(val), cimag(val));

    return result;
}

static PyObject *
doki_registry_apply (PyObject *self, PyObject *args)
{
    PyObject *raw_val, *state_capsule, *gate_capsule,
             *target_set, *control_set, *acontrol_set;
    void *raw_state, *raw_gate;
    struct state_vector *state, *new_state;
    struct qgate *gate;
    unsigned char exit_code;
    unsigned int num_targets, num_controls, num_anticontrols, i;
    unsigned int *targets, *controls, *anticontrols;

    if (!PyArg_ParseTuple(args, "OOOOO", &state_capsule, &gate_capsule,
                          &target_set, &control_set, &acontrol_set)) {
        PyErr_SetString(DokiError, "Syntax: apply(registry, gate, target_set, control_set, anticontrol_set)");
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

    if (!PySet_Check(target_set)) {
        PyErr_SetString(DokiError, "target_set must be a set");
        return NULL;
    }

    num_targets = PySet_Size(target_set);
    if (num_targets != gate->num_qubits) {
        PyErr_SetString(DokiError, "Wrong number of targets specified for that gate");
        return NULL;
    }

    num_controls = 0;
    if (PySet_Check(control_set)) {
        num_controls = PySet_Size(control_set);
    }
    else if (control_set != Py_None) {
        PyErr_SetString(DokiError, "control_set must be a set or None");
        return NULL;
    }

    num_anticontrols = 0;
    if (PySet_Check(acontrol_set)) {
        num_anticontrols = PySet_Size(acontrol_set);
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
    if (num_controls > 0) {
        controls = MALLOC_TYPE(num_controls, unsigned int);
        if (controls == NULL) {
            PyErr_SetString(DokiError, "Failed to allocate control array");
            return NULL;
        }
    }
    if (num_anticontrols > 0) {
        anticontrols = MALLOC_TYPE(num_anticontrols, unsigned int);
        if (anticontrols == NULL) {
            PyErr_SetString(DokiError, "Failed to allocate anticontrol array");
            return NULL;
        }
    }

    exit_code = 0;
    for (i = 0; i < num_targets; i++) {
        raw_val = PySet_Pop(target_set);
        if(!PyLong_Check(raw_val)) {
            PyErr_SetString(DokiError, "target_set must be a set qubit ids (unsigned integers)");
            break;
        }
        targets[i] = PyLong_AsLong(raw_val);
        if (targets[i] >= state->num_qubits) {
            PyErr_SetString(DokiError, "Target qubit out of range");
            break;
        }
    }

    for (i = 0; i < num_controls; i++) {
        raw_val = PySet_Pop(control_set);
        if(!PyLong_Check(raw_val)) {
            PyErr_SetString(DokiError, "control_set must be a set qubit ids (unsigned integers)");
            break;
        }
        controls[i] = PyLong_AsLong(raw_val);
        if (controls[i] >= state->num_qubits) {
            PyErr_SetString(DokiError, "Control qubit out of range");
            break;
        }
    }

    for (i = 0; i < num_anticontrols; i++) {
        raw_val = PySet_Pop(acontrol_set);
        if(!PyLong_Check(raw_val)) {
            PyErr_SetString(DokiError, "anticontrol_set must be a set qubit ids (unsigned integers)");
            break;
        }
        anticontrols[i] = PyLong_AsLong(raw_val);
        if (anticontrols[i] >= state->num_qubits) {
            PyErr_SetString(DokiError, "Anticontrol qubit out of range");
            break;
        }
    }

    if (exit_code == 0) {
        new_state = MALLOC_TYPE(1, struct state_vector);
        if (new_state == NULL) {
            PyErr_SetString(DokiError, "Failed to allocate new state structure");
            return NULL;
        }
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

    if (!PyArg_ParseTuple(args, "OO", &capsule1, &capsule2)) {
        PyErr_SetString(DokiError, "Syntax: join(most_registry, least_registry)");
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
    exit_code = join(result, state1, state2);

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
        PyErr_SetString(DokiError, "Failed to get/set a value");
    }
    else if (exit_code != 0) {
        PyErr_SetString(DokiError, "Unknown error when applying gate");
    }

    if (exit_code != 0) {
        return NULL;
    }

    return PyCapsule_New((void*) result, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
}

static PyObject *
doki_registry_measure (PyObject *self, PyObject *args)
{
    PyObject *capsule, *py_measured_val, *result, *new_capsule;
    void *raw_state;
    struct state_vector *state, *new_state, *aux;
    NATURAL_TYPE mask;
    unsigned int i, curr_id, initial_num_qubits;
    _Bool measure_id, measured_val;
    unsigned char exit_code;

    if (!PyArg_ParseTuple(args, "OK", &capsule, &mask)) {
        PyErr_SetString(DokiError, "Syntax: measure(registry, mask)");
        return NULL;
    }

    raw_state = PyCapsule_GetPointer(capsule, "qsimov.doki.state_vector");
    if (raw_state == NULL) {
        PyErr_SetString(DokiError, "NULL pointer to registry");
        return NULL;
    }
    state = (struct state_vector*) raw_state;
    initial_num_qubits = state->num_qubits;
    result = PyList_New(initial_num_qubits);

    exit_code = 0;
    aux = state;
    new_state = state;
    for (i = 0; i < initial_num_qubits; i++) {
        if (exit_code != 0) {
            break;
        }
        curr_id = initial_num_qubits - i - 1;
        measure_id = mask & (NATURAL_ONE << curr_id);
        py_measured_val = Py_None;
        if (measure_id) {
            new_state = MALLOC_TYPE(1, struct state_vector);
            if (new_state == NULL) {
                PyErr_SetString(DokiError, "Failed to allocate new state structure");
            }
            exit_code = measure(aux, &measured_val, curr_id, new_state);
            py_measured_val = measured_val ? Py_True : Py_False;
            if (aux != state) {
                state_clear(aux);
                free(aux);
            }
            aux = new_state;
        }
        PyList_SET_ITEM(result, i, py_measured_val);
    }
    if (exit_code != 0) {
        switch (exit_code) {
            case 1:
                PyErr_SetString(DokiError, "Not on this computation node");
                break;
            case 2:
                PyErr_SetString(DokiError, "Tried to access element out of bounds");
                break;
            case 3:
                PyErr_SetString(DokiError, "Failed to initialize new state chunk");
                break;
            case 4:
                PyErr_SetString(DokiError, "Failed to allocate new state chunk");
                break;
            case 5:
                PyErr_SetString(DokiError, "[BUG] THIS SHOULD NOT HAPPEN. Failed to set first value to 1");
                break;
            case 6:
                PyErr_SetString(DokiError, "Failed to allocate new state vector structure");
                break;
            case 7:
                PyErr_SetString(DokiError, "[BUG] THIS SHOULD NOT HAPPEN. Failed to allocate new state structure");
                break;
            case 8:
                PyErr_SetString(DokiError, "Failed to get/set a value while collapsing");
                break;
            default:
                PyErr_SetString(DokiError, "Unknown error!");
        }
        return NULL;
    }

    new_capsule = PyCapsule_New((void*) new_state, "qsimov.doki.state_vector",
                         &doki_registry_destroy);
    return PyTuple_Pack(2, new_capsule, result);
}
