#!/bin/bash
# Pycee++ Bootstrap Script
# Compiles the self-hosting compiler using the C implementation

set -e  # Exit on any error

echo "🌱 Starting Pycee++ bootstrap process..."
echo "🚀 Step 1: Building initial C compiler"

# Build the C compiler
make clean
make

echo "✅ C compiler built successfully"
echo "🚀 Step 2: Compiling Pycee++ standard library"

# Compile stdlib to bytecode
bin/pycee build lib/stdlib.pyc

echo "✅ Standard library compiled"
echo "🚀 Step 3: Compiling the self-hosting compiler"

# Compile the compiler written in Pycee++
bin/pycee build lib/compiler.pyc

echo "✅ Pycee++ compiler compiled to bytecode"
echo "🚀 Step 4: Using bytecode compiler to compile itself"

# Use the bytecode compiler to compile the compiler source again
bin/pycee vm lib/compiler.pycb lib/compiler_source.pyc lib/compiler_stage1.c

echo "✅ Self-compilation complete"
echo "🚀 Step 5: Building the native self-hosting compiler"

# Compile the generated C code to native binary
gcc lib/compiler_stage1.c runtime/pycee_rt.c -Iinclude -lm -o bin/pycee_stage1

echo "✅ Stage 1 native compiler built"
echo "🚀 Step 6: Final self-hosting compilation"

# Use stage1 compiler to compile the compiler source one more time
bin/pycee_stage1 lib/compiler_source.pyc bin/pycee_final.c

# Build the final compiler
gcc bin/pycee_final.c runtime/pycee_rt.c -Iinclude -lm -o bin/pycee

echo "✅ Final self-hosting compiler built"
echo "🚀 Step 7: Verifying the compiler"

# Test the final compiler
bin/pycee --version
bin/pycee run samples/hello.pyc

echo "✨ Bootstrap complete! Pycee++ is now self-hosting."
echo "🎉 Final compiler located at: bin/pycee"
echo ""
echo "🚀 Next steps:"
echo "   pycee run samples/game.pyc          # Run game in interpreted mode"
echo "   pycee compile samples/game.pyc      # Compile to native binary"
echo "   pycee compile-ui samples/login.pyc  # Generate UI for all platforms"