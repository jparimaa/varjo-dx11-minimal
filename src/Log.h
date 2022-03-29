#pragma once

#include <Varjo.h>

#include <comdef.h>

#include <iostream>
#include <string>

inline void printError(const std::string& msg)
{
    std::cerr << "ERROR: " << msg << "\n";
}

class ConsoleOutput
{
public:
    ConsoleOutput()
    {
        AllocConsole();
        freopen_s(&m_consoleStream, "CONOUT$", "w", stdout);
        freopen_s(&m_consoleStream, "CONOUT$", "w", stderr);
    }

    ~ConsoleOutput()
    {
        fclose(m_consoleStream);
        FreeConsole();
    }

private:
    FILE* m_consoleStream = nullptr;
};