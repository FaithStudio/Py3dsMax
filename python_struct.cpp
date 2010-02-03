/*
	\file		python_struct.cpp

	\remarks	This file defines the 3dsMax struct called python
*/

#include "imports.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <import.h>
#include <graminit.h>
#include <pythonrun.h>

// setup the scripter export symbols
#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "defextfn.h"
#include "definsfn.h"

#include "macros.h"
#include "wrapper.h"

// Define the python struct in 3dsMax
def_struct_primitive( import,			python,			"import" );
def_struct_primitive( reload,			python,			"reload" );
def_struct_primitive( run,				python,			"run" );
def_struct_primitive( exec,				python,			"exec" );

// python.import function: import a python module to maxscript
Value*
import_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct
	check_arg_count( python.import, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT( two_value_locals( mxs_check, mxs_return ) );

	// Step 3: convert the maxscript value
	MXS_EVAL( arg_list[0], vl.mxs_check );

	// Step 4: calculate the python module name to import
	char* module_name = NULL;
	try { module_name = vl.mxs_check->to_string(); }
	MXS_CATCHERRORS();

	// Step 5: import the module
	if ( module_name ) {
		PyObject* module = PyImport_ImportModule( module_name );
		PY_CLEARERRORS();

		vl.mxs_return = ( module ) ? ObjectWrapper::intern( module ) : &undefined;
	}
	else {
		mprintf( "python.import() error: importing modules must be done with a string value\n" );
		vl.mxs_return = &undefined;
	}
	MXS_RETURN( vl.mxs_return );
}

// python.reload function: reload an existing module in maxscript
Value*
reload_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.reload, 1, count );

	// Step 2: evaluate the input item
	Value* mxs_check = NULL;
	MXS_EVAL( arg_list[0], mxs_check );
	
	// Step 3: make sure the item is a proper type
	if ( is_objectwrapper(mxs_check) ) {
		PyImport_ReloadModule( ((ObjectWrapper*) mxs_check)->object() );
		PY_CLEARERRORS();
	}
	else { mprintf( "python.reload() error: you need to supply a valid python module to reload\n" ); }

	return &ok;	
}

// python.run function: run a python file from maxscript
Value*
run_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.run, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT(one_value_local(mxs_filename));
	MXS_EVAL( arg_list[0], vl.mxs_filename );

	// Step 2: create a python file based on the filename
	char* filename		= vl.mxs_filename->to_string();
	PyObject* py_file	= PyFile_FromString( filename, "r" );

	// Step 3: check to make sure the file exists
	if ( !py_file ) {
		return &false_value;
	}

	// Step 4: run the file
	PyRun_SimpleFile( PyFile_AsFile(py_file), filename );
	PY_CLEARERRORS();

	// Step 5: cleanup the memory
	Py_XDECREF( py_file );
	MXS_CLEANUP();
	
	return &true_value;
}

// python.exec function: execute a python command through a maxscript string
Value*
exec_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.exec, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT(one_value_local(mxs_command));
	MXS_EVAL( arg_list[0], vl.mxs_command );

	// Step 2: create a python file based on the filename
	char* command	= NULL;
	try { command	= vl.mxs_command->to_string(); }
	MXS_CATCHERRORS();

	// Step 3: check to make sure the command is valid
	if ( !command ) {
		return &false_value;
	}

	// Step 4: run the command
	PyRun_SimpleString( command );
	PY_CLEARERRORS();

	// Step 5: cleanup the memory
	MXS_CLEANUP();
	
	return &ok;
}