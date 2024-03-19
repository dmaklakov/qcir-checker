# qcir-checker

Please use the .pdf to check the grammar, small example is provided in `small.qcir`.
To compile a project, use:`

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

During development, use:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j4
cd ..
ln build/compile_commands.json
```

Your editor can then use the compile_commands.json for code completion.

To run the programm, use:

```bash
qcir.exe [-cleansed] [-light] FILE
```
