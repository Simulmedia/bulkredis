#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static PyObject *exec_bulk_redis(PyObject *self, PyObject *args) {
	PyObject *dict, *key, *value_dict, *value, *entry, *tmp;
	Py_ssize_t pos = 0, sub_pos = 0;
	char *key_str, *value_str, *entry_str;
	Py_ssize_t key_len, value_len, entry_len;
	const char *file_name, *host, *port;
	char exec[1024];
	FILE *file;
	int ret;

	if (!PyArg_ParseTuple(args, "sssO!", &host, &port, &file_name, &PyDict_Type, &dict))
		return NULL;

	file = fopen(file_name, "w");
	if (file == NULL) {
		fprintf(stderr, "Failed to open '%s' for writing: ", file_name);
		perror("");
		/* Could also raise an exception. */
		return Py_BuildValue("i", -1);
	}

	while (PyDict_Next(dict, &pos, &key, &value_dict)) {

		tmp = PyObject_Str(key);
		key_str = PyString_AsString(tmp);
		key_len = PyString_Size(tmp);
		Py_DECREF(tmp);

		while (PyDict_Next(value_dict, &sub_pos, &value, &entry)) {

			tmp = PyObject_Str(value);
			value_str = PyString_AsString(tmp);
			value_len = PyString_Size(tmp);
			Py_DECREF(tmp);

			if (entry == Py_None) {
				fprintf(file, "*3\r\n$4\r\nSADD\r\n$%lu\r\n%s\r\n$%lu\r\n%s\r\n",
					key_len, key_str,
					value_len, value_str);
			} else {
				tmp = PyObject_Str(entry);
				entry_str = PyString_AsString(tmp);
				entry_len = PyString_Size(tmp);
				Py_DECREF(tmp);

				if (PyString_Check(entry)) {
					fprintf(file, "*4\r\n$4\r\nHSET\r\n$%lu\r\n%s\r\n$%lu\r\n%s\r\n$%lu\r\n%s\r\n",
						key_len, key_str,
						entry_len, entry_str,
						value_len, value_str);
				} else {
					fprintf(file, "*4\r\n$4\r\nZADD\r\n$%lu\r\n%s\r\n$%lu\r\n%s\r\n$%lu\r\n%s\r\n",
						key_len, key_str,
						entry_len, entry_str,
						value_len, value_str);
				}
			}
		}
		sub_pos = 0;
	}

	fclose(file);

	snprintf(exec, 1024, "cat %s | redis-cli -h %s -p %s --pipe > /dev/null", file_name, host, port);
	ret = system(exec);

	return Py_BuildValue("i", ret);
}

static PyMethodDef BulkRedisMethods[] = {
	{"execute", exec_bulk_redis, METH_VARARGS, "Bulk load dict."},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initbulkredis(void) {
	(void) Py_InitModule("bulkredis", BulkRedisMethods);
}
