/* This file contains Python support for the perfstubs API.
 */

// this line has to go first
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define PERFSTUBS_USE_TIMERS
#include "timer.h"
#include <stack>

std::stack<std::string>& timerStack(void) {
    static thread_local std::stack<std::string> theStack;
    return theStack;
}

extern "C" {

static PyObject* perfstubs_initialize([[maybe_unused]] PyObject *self, [[maybe_unused]] PyObject *args) {
    ps_initialize_();
    // we return no useful argument, so return the Python None object.
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* perfstubs_finalize([[maybe_unused]] PyObject *self, [[maybe_unused]] PyObject *args) {
    ps_finalize_();
    // we return no useful argument, so return the Python None object.
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* perfstubs_get_python_version([[maybe_unused]] PyObject *self, [[maybe_unused]] PyObject* noarg) {
  int major = PY_MAJOR_VERSION;
  int minor = PY_MINOR_VERSION;
  int micro = PY_MICRO_VERSION;
  PyObject * result = Py_BuildValue("iii", major, minor, micro);
  return result;
}

static PyObject* perfstubs_start([[maybe_unused]] PyObject *self, PyObject *args) {
    const char *timer_name;
    const char *file_name;
    uint64_t line_number;
    if (PyArg_ParseTuple(args, "ssl", &timer_name, &file_name, &line_number)) {
        char * tmpstr = ps_make_timer_name_(file_name, timer_name, line_number);
        ps_start_string_(tmpstr);
        timerStack().push(std::string(tmpstr));
        free(tmpstr);
    }
    // we return no useful argument, so return the Python None object.
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* perfstubs_stop([[maybe_unused]] PyObject *self, [[maybe_unused]] PyObject *args) {
/* Ideally, we could construct the name, but the line nunber we get is the return
   statement, not the start of the function. So we maintain a thread_local stack
   and just pop-and-stop the current timer on this thread. */
#if 0
    const char *timer_name;
    const char *file_name;
    uint64_t line_number;
    if (PyArg_ParseTuple(args, "ssl", &timer_name, &file_name, &line_number)) {
        char * tmpstr = ps_make_timer_name_(file_name, timer_name, 1L);
        ps_stop_string_(tmpstr);
        free(tmpstr);
    }
#else
    if (timerStack().size() > 0) {
        auto timer = timerStack().top();
        ps_stop_string_(timer.c_str());
        timerStack().pop();
    }
#endif
    // we return no useful argument, so return the Python None object.
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PerfStubsMethods[] = {
    {"get_python_version", perfstubs_get_python_version, METH_VARARGS,
     "Get compiled Python version for libperfstubs.so."},
    {"initialize", perfstubs_initialize, METH_VARARGS,
     "Initialize PerfStubs."},
    {"finalize", perfstubs_finalize, METH_VARARGS,
     "Finalize PerfStubs."},
    {"start", perfstubs_start, METH_VARARGS,
     "Start a PerfStubs timer."},
    {"stop", perfstubs_stop, METH_VARARGS,
     "Stop a PerfStubs timer."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyModuleDef_Slot PerfStubsSlots[] = {
    {0, NULL}
};

static struct PyModuleDef perfstubsmodule = {
    PyModuleDef_HEAD_INIT,
    "perfstubs",   /* name of module */
    NULL, /* module documentation, may be NULL */ // fixme!
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    PerfStubsMethods /* the m_methods object, required */
#if 0
    PerfStubsSlots, /* the m_slots object, optional */
    NULL, /* the m_traverse function, optional */
    NULL, /* the m_clear function, optional */
    NULL /* the m_clear function, optional */
#endif
};

PyMODINIT_FUNC PyInit_perfstubs(void) {
    return PyModule_Create(&perfstubsmodule);
}

} // extern "C"


