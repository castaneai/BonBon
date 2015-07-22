#include "precompiled_header.h"

std::thread* output_thread = nullptr;
std::atomic_bool running = false;

void output_thread_func(std::ostream& output, const int& channel, const std::string& bon_dll_path, bool decode_enabled = true)
{
    #pragma region startup

    auto bon_dll = LoadLibraryA(bon_dll_path.c_str());
    if (bon_dll == nullptr) {
        std::cerr << "LoadLibrary(" << bon_dll_path << ") failed." << std::endl;
        return;
    }
    auto bon = reinterpret_cast<IBonDriver2*(*)()>(GetProcAddress(bon_dll, "CreateBonDriver"))();
    if (bon == nullptr) {
        std::cerr << "CreateBonDriver() failed." << std::endl;
        return;
    }
    if (!bon->OpenTuner()) {
        std::cerr << "OpenTuner() failed." << std::endl;
        return;
    }
    if (!bon->SetChannel(0, channel)) {
        std::cerr << "SetChannel(" << channel << ") failed." << std::endl;
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto decoder_dll = LoadLibraryA("B25Decoder.dll");
    if (decoder_dll == nullptr) {
        std::cerr << "LoadLibrary(B25Decoder.dll) failed." << std::endl;
        return;
    }
    auto decoder = reinterpret_cast<IB25Decoder2*(*)()>(GetProcAddress(decoder_dll, "CreateB25Decoder2"))();
    if (!decoder->Initialize()) {
        std::cerr << "B25Decoder->Initialize() failed." << std::endl;
        return;
    }
    decoder->DiscardNullPacket(true);
    decoder->DiscardScramblePacket(false);

    #pragma endregion

    bon->PurgeTsStream();
    running = true;
    while (running) {
        BYTE* read_buffer = nullptr;
        DWORD read_size = 0;
        DWORD remain = 0;
        if (bon->GetTsStream(&read_buffer, &read_size, &remain) && read_buffer != nullptr && read_size > 0) {
            if (decode_enabled) {
                BYTE* decoded_buffer = nullptr;
                DWORD decoded_size = 0;
                if (decoder->Decode(read_buffer, read_size, &decoded_buffer, &decoded_size) && decoded_buffer != nullptr && decoded_size > 0) {
                    output.write(reinterpret_cast<const char*>(decoded_buffer), decoded_size);
                }
            }
            else {
                output.write(reinterpret_cast<const char*>(read_buffer), read_size);
            }
            if (remain == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    #pragma region cleanup

    BYTE* decoded_buffer = nullptr;
    DWORD decoded_size = 0;
    if (decoder->Flush(&decoded_buffer, &decoded_size) && decoded_buffer != nullptr && decoded_size > 0) {
        output.write(reinterpret_cast<const char*>(decoded_buffer), decoded_size);
    }
    bon->PurgeTsStream();
    bon->CloseTuner();
    bon->Release();
    FreeLibrary(bon_dll);
    FreeLibrary(decoder_dll);

    #pragma endregion
}

void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGBREAK) {
        running = false;
        if (output_thread != nullptr) {
            output_thread->join();
            delete output_thread;
        }
    }
    exit(signal);
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_signal);
    signal(SIGBREAK, handle_signal);
    if (argc < 2) {
        std::cout << "Usage: BonBon <channel_number> [<bon_dll_path>]" << std::endl;
        return 0;
    }

    #ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
    #endif

    const auto channel = (argc == 2) ? atoi(argv[1]) : 3;
    const auto bon_dll_path = std::string((argc > 2) ? argv[2] : "BonDriver_PT3-T.dll");

    output_thread = new std::thread([channel, bon_dll_path]() {
        output_thread_func(std::cout, channel, bon_dll_path);
    });
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}