// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "LauncherPaneContent.h"
#include "LauncherPaneContent.g.cpp"
#include "LaunchRequestedArgs.g.cpp"
#include "GitCommitItem.g.cpp"

#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <Shobjidl.h>
#include <ShlObj.h>
#include <filesystem>
#include <sstream>
#include "Utils.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;
using namespace winrt::Microsoft::Terminal::Settings::Model;

namespace winrt::TerminalApp::implementation
{
    LauncherPaneContent::LauncherPaneContent()
    {
        InitializeComponent();

        // Initialize git commits collection
        _gitCommits = winrt::single_threaded_observable_vector<TerminalApp::GitCommitItem>();

        // Load images from file system (for portable/unpackaged apps)
        try
        {
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
            std::filesystem::path assetsDir = exeDir / L"Assets";

            // Load Claude image
            auto claudePath = assetsDir / L"claude-code.png";
            if (std::filesystem::exists(claudePath))
            {
                BitmapImage claudeBitmap;
                claudeBitmap.UriSource(Uri(L"file:///" + claudePath.wstring()));
                _claudeImage().Source(claudeBitmap);
            }

            // Load Codex image
            auto codexPath = assetsDir / L"codex.png";
            if (std::filesystem::exists(codexPath))
            {
                BitmapImage codexBitmap;
                codexBitmap.UriSource(Uri(L"file:///" + codexPath.wstring()));
                _codexImage().Source(codexBitmap);
            }

            // Load Gemini image
            auto geminiPath = assetsDir / L"Gemini.png";
            if (std::filesystem::exists(geminiPath))
            {
                BitmapImage geminiBitmap;
                geminiBitmap.UriSource(Uri(L"file:///" + geminiPath.wstring()));
                _geminiImage().Source(geminiBitmap);
            }
        }
        catch (...)
        {
            // Silently ignore image loading errors
        }
    }

    winrt::Windows::UI::Xaml::FrameworkElement LauncherPaneContent::GetRoot()
    {
        return *this;
    }

    void LauncherPaneContent::UpdateSettings(const CascadiaSettings& settings)
    {
        _settings = settings;
    }

    winrt::Windows::Foundation::Size LauncherPaneContent::MinimumSize()
    {
        return { 200, 200 };
    }

    void LauncherPaneContent::Focus(winrt::Windows::UI::Xaml::FocusState reason)
    {
        _selectFolderButton().Focus(reason);
    }

    void LauncherPaneContent::Close()
    {
        CloseRequested.raise(*this, nullptr);
    }

    INewContentArgs LauncherPaneContent::GetNewTerminalArgs(BuildStartupKind /*kind*/) const
    {
        return nullptr;
    }

    winrt::hstring LauncherPaneContent::Icon() const
    {
        return L"\xE756"; // Developer icon
    }

    winrt::Windows::UI::Xaml::Media::Brush LauncherPaneContent::BackgroundBrush()
    {
        return nullptr;
    }

    safe_void_coroutine LauncherPaneContent::_selectFolderClick(const IInspectable&, const RoutedEventArgs&)
    {
        auto lifetime = get_strong();

        const auto parentHwnd = _hostingHwnd;
        winrt::hstring previousFolder = SelectedFolder();

        auto folder = co_await OpenFilePicker(parentHwnd, [previousFolder](auto&& dialog) {
            static constexpr winrt::guid clientGuidLauncherFolderPicker{ 0x7b3f0c2a, 0x1d5e, 0x4a8b, { 0x9c, 0x6f, 0x3e, 0x8d, 0x5a, 0x2b, 0x7c, 0x41 } };
            THROW_IF_FAILED(dialog->SetClientGuid(clientGuidLauncherFolderPicker));

            DWORD dwOptions;
            THROW_IF_FAILED(dialog->GetOptions(&dwOptions));
            THROW_IF_FAILED(dialog->SetOptions(dwOptions | FOS_PICKFOLDERS));

            if (!previousFolder.empty())
            {
                try
                {
                    auto folderShellItem{ winrt::capture<IShellItem>(&SHCreateItemFromParsingName, previousFolder.c_str(), nullptr) };
                    dialog->SetFolder(folderShellItem.get());
                }
                CATCH_LOG();
            }
        });

        if (!folder.empty())
        {
            SelectedFolder(folder);
        }
    }

    void LauncherPaneContent::_claudeClick(const IInspectable&, const RoutedEventArgs&)
    {
        _launchWithCommand(L"cc\r");
    }

    void LauncherPaneContent::_codexClick(const IInspectable&, const RoutedEventArgs&)
    {
        _launchWithCommand(L"codex\r");
    }

    void LauncherPaneContent::_geminiClick(const IInspectable&, const RoutedEventArgs&)
    {
        _launchWithCommand(L"gemini\r");
    }

    void LauncherPaneContent::_launchWithCommand(const winrt::hstring& command)
    {
        winrt::hstring workingDir = SelectedFolder();

        // If no folder selected, use user's home directory
        if (workingDir.empty())
        {
            wchar_t userProfile[MAX_PATH];
            if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) > 0)
            {
                workingDir = userProfile;
            }
        }

        auto args = winrt::make<LaunchRequestedArgs>();
        args.WorkingDirectory(workingDir);
        args.Command(command);
        LaunchRequested.raise(*this, args);
    }

    safe_void_coroutine LauncherPaneContent::UpdateGitLog(const winrt::hstring& workingDirectory)
    {
        auto lifetime = get_strong();

        _gitCommits.Clear();

        if (workingDirectory.empty())
        {
            _gitCommitsControl().ItemsSource(_gitCommits);
            co_return;
        }

        std::wstring workDir{ workingDirectory };

        // Switch to background thread for blocking I/O
        co_await winrt::resume_background();

        std::wstring cmdLine = L"cmd.exe /c git -C \"" + workDir + L"\" log --pretty=format:\"%ad  %h|%s\" --date=format:\"%Y-%m-%d %H:%M:%S\"";

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE hReadPipe, hWritePipe;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        {
            co_await wil::resume_foreground(Dispatcher());
            _gitCommitsControl().ItemsSource(_gitCommits);
            co_return;
        }

        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOW si = { sizeof(STARTUPINFOW) };
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi = {};

        std::wstring cmdBuffer = cmdLine;
        if (!CreateProcessW(nullptr, cmdBuffer.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, workDir.c_str(), &si, &pi))
        {
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            co_await wil::resume_foreground(Dispatcher());
            _gitCommitsControl().ItemsSource(_gitCommits);
            co_return;
        }

        CloseHandle(hWritePipe);

        std::string output;
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        CloseHandle(hReadPipe);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Parse on background thread
        std::vector<std::pair<std::wstring, std::wstring>> commits;
        if (!output.empty())
        {
            std::wstring wOutput;
            int wideLen = MultiByteToWideChar(CP_UTF8, 0, output.c_str(), static_cast<int>(output.size()), nullptr, 0);
            if (wideLen > 0)
            {
                wOutput.resize(wideLen);
                MultiByteToWideChar(CP_UTF8, 0, output.c_str(), static_cast<int>(output.size()), wOutput.data(), wideLen);
            }
            std::wistringstream stream(wOutput);
            std::wstring line;
            while (std::getline(stream, line))
            {
                if (line.empty()) continue;
                auto separatorPos = line.find(L'|');
                if (separatorPos != std::wstring::npos)
                {
                    commits.emplace_back(line.substr(0, separatorPos), line.substr(separatorPos + 1));
                }
            }
        }

        // Switch back to UI thread
        co_await wil::resume_foreground(Dispatcher());

        for (const auto& [dateTimeAndHash, message] : commits)
        {
            _gitCommits.Append(winrt::make<GitCommitItem>(winrt::hstring(dateTimeAndHash), winrt::hstring(message)));
        }
        _gitCommitsControl().ItemsSource(_gitCommits);
    }
}
