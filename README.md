# qcir-checker

Please use the .pdf to check the grammar, small example is provided in `small.qcir`.
To compile a project, use:`

```bash
g++ Scanner.cpp Parser.cpp ParserLight.cpp qcir.cpp -o qcir.exe
```

To run the programm, use:

```bash
qcir.exe FILE [-cleansed] [-light]
```
