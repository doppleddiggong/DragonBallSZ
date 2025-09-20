#include "Shared/FLogWriter.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "GenericPlatform/GenericPlatformProcess.h"


FLogWriter& FLogWriter::Get()
{
    static FLogWriter Instance;
    return Instance;
}

FLogWriter::FLogWriter()
{
    if (!FPlatformProcess::SupportsMultithreading())
    {
        return;
    }

    const FString TimeString = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
    const FString LogFileName = FString::Printf(TEXT("CoffeeLog-%s.log"), *TimeString);
    const FString LogFilePath = FPaths::ProjectLogDir() / LogFileName;

    LogFile = IFileManager::Get().CreateFileWriter(*LogFilePath, FILEWRITE_Append);

    if (!LogFile)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open log file at: %s"), *LogFilePath);
    }
}

FLogWriter::~FLogWriter()
{
    if (LogFile)
    {
        LogFile->Close();
        delete LogFile;
        LogFile = nullptr;
    }
}

void FLogWriter::WriteLog(const FString& Message)
{
    if (!LogFile)
    {
        return;
    }

    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
    const FString LogLine = FString::Printf(TEXT("[%s] %s\n"), *Timestamp, *Message);

    FTCHARToUTF8 Converter(*LogLine);
    LogFile->Serialize(const_cast<void*>(static_cast<const void*>(Converter.Get())), Converter.Length());
    LogFile->Flush(); // TODO: Optionalize or buffer later
}