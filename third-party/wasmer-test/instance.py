import sys
# Wasmer will let you easily run WebAssembly module in a Python host.
#
# This example illustrates the basics of using Wasmer through a “Hello
# World”-like project:
#
#   1. How to load a WebAssembly module as bytes,
#   2. How to compile the mdule,
#   3. How to create an instance of the module.
#
# You can run the example directly by executing in Wasmer root:
#
# ```shell
# $ python examples/instance.py
# ```
#
# Ready?

from wasmer import engine, wat2wasm, Store, Module, Instance, ImportObject, Function, FunctionType, Type, Instance
from wasmer_compiler_cranelift import Compiler

# Let's declare the Wasm module.
#
# We are using the text representation of the module here but you can
# also load `.wasm` files using the `open` function.
# wasm_bytes = wat2wasm(
#     """
#     (module
#       (type $add_one_t (func (param i32) (result i32)))
#       (func $add_one_f (type $add_one_t) (param $value i32) (result i32)
#         local.get $value
#         i32.const 1
#         i32.add)
#       (export "add_one" (func $add_one_f)))
#     """
# )

wasm_dir = "../../example/" + str(sys.argv[1]) + ".wasm"
wasm_bytes = open(wasm_dir, 'rb').read()

# Create a store. Engines and compilers are explained in other
# examples.
store = Store(engine.JIT(Compiler))

# Let's compile the Wasm module.
module = Module(store, wasm_bytes)

# Imports
def printInt(x: int):
    print("Output: {}".format(x))

def getA()->int:
    return 22

def getB()->int:
    return 33

printInt_host_function = Function(store, printInt)
getA_host_function = Function(store, getA)
getB_host_function = Function(store, getB)

import_object = ImportObject()
import_object.register(
    "env",
    {
        "printInt": printInt_host_function,
        "getA": getA_host_function,
        "getB": getB_host_function
    }
)

# Let's instantiate the module!
instance = Instance(module, import_object)

# We now have an instance ready to be used.
#
# From an `Instance` we can retrieve any exported entities. Each of
# these entities is covered in others examples.
#
# Here we are retrieving the exported function. We won't go into
# details here as the main focus of this example is to show how to
# create an instance out of a Wasm module and have basic interactions
# with it.
# assert instance.exports.add_one(1) == 2
print(instance.exports.main())
