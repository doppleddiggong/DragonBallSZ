
#pragma once

#include "CoreMinimal.h"

class FArchive;

class COFFEELIBRARY_API FLogWriter
{
public:
    FLogWriter();
    ~FLogWriter();

    void WriteLog(const FString& Message);

    static FLogWriter& Get();

private:
    FArchive* LogFile = nullptr;
};
