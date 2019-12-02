@echo off
cl /nologo /EHsc /Ox /MT /W3 /WX /D_CRT_SECURE_NO_WARNINGS User32.lib Shell32.lib winclip.cpp /Fewinclip.exe
