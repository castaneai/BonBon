#include "precompiled_header.h"

HMODULE bonDriverDll = nullptr;
HMODULE b25DecoderDll = nullptr;
IBonDriver2* bonDriver = nullptr;
IB25Decoder2* b25Decoder = nullptr;
FILE* tsOutputStream = nullptr;

bool startup()
{
    bonDriverDll = LoadLibraryA("BonDriver_PT3-T.dll");
    if (bonDriverDll == nullptr) {
        fprintf(stderr, "LoadLibrary(BonDriver_PT3-T.dll) failed.\n");
        return false;
    }
    bonDriver = dynamic_cast<IBonDriver2*>(((IBonDriver*(*)())GetProcAddress(bonDriverDll, "CreateBonDriver"))());
    if (bonDriver == nullptr) {
        fprintf(stderr, "CreateBonDriver() failed.\n");
        return false;
    }
    if (!bonDriver->OpenTuner()) {
        fprintf(stderr, "OpenTuner() failed.\n");
        return false;
    }

    b25DecoderDll = LoadLibraryA("B25Decoder.dll");
    if (b25DecoderDll == nullptr) {
        fprintf(stderr, "LoadLibrary(B25Decoder.dll) failed.\n");
        return false;
    }
    b25Decoder = ((IB25Decoder2*(*)())GetProcAddress(b25DecoderDll, "CreateB25Decoder2"))();
    if (!b25Decoder->Initialize()) {
        fprintf(stderr, "B25Decoder initialize failed.\n");
        return false;
    }
    b25Decoder->DiscardNullPacket(true);
    b25Decoder->DiscardScramblePacket(false);
    return true;
}

void cleanup()
{
    if (tsOutputStream != nullptr) fclose(tsOutputStream);
    if (b25Decoder != nullptr) b25Decoder->Release();
    if (bonDriver != nullptr) {
        bonDriver->CloseTuner();
        bonDriver->Release();
    }
    if (bonDriverDll != nullptr) FreeLibrary(bonDriverDll);
    if (b25DecoderDll != nullptr) FreeLibrary(b25DecoderDll);
}

void signalHandler(int signal)
{
    if (signal == SIGINT) {
        cleanup();
    }
    exit(signal);
}

void writeTS(FILE* const& fp)
{
    BYTE* tsData = nullptr;
    DWORD gotDataSize = 0;
    DWORD remainDataSize = 0;
    if (bonDriver->GetTsStream(&tsData, &gotDataSize, &remainDataSize) &&
        gotDataSize > 0) {
        BYTE* decodedTsData = nullptr;
        DWORD decodedDataSize = 0;
        if (b25Decoder->Decode(tsData, gotDataSize, &decodedTsData, &decodedDataSize) &&
            decodedDataSize > 0) {
            fwrite(decodedTsData, decodedDataSize, 1, fp);
        }
        if (remainDataSize == 0) {
            Sleep(100);
        }
    }
}

int main(int argc, char** argv)
{
    signal(SIGINT, signalHandler);

    if (!startup()) {
        fprintf(stderr, "startup() failed.\n");
        return -1;
    }

    const auto channel = (argc == 2) ? atoi(argv[1]) : 3;
    tsOutputStream = (argc == 3) ? fopen(argv[2], "wb") : stdout;
    if (tsOutputStream == nullptr) {
        fprintf(stderr, "fopen(%s) failed.\n", (argc == 3) ? argv[2] : "stdout");
    }
    if (tsOutputStream == stdout) {
        _setmode(_fileno(stdout), _O_BINARY);
    }

    if (!bonDriver->SetChannel(0, channel)) {
        fprintf(stderr, "SetChannel(0, %d) failed.\n", channel);
        return -1;
    }
    Sleep(1000);

    bonDriver->PurgeTsStream();
    while (true) {
        writeTS(tsOutputStream);
    }
    return 0;
}