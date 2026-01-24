// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "LauncherPaneContent.h"
#include "LauncherPaneContent.g.cpp"
#include "LaunchRequestedArgs.g.cpp"
#include "GitCommitItem.g.cpp"

#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <Shobjidl.h>
#include <filesystem>
#include <sstream>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;
using namespace winrt::Windows::Storage::Pickers;
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

    fire_and_forget _selectFolderAsync(winrt::weak_ref<LauncherPaneContent> weakThis, HWND hwnd)
    {
        auto strongThis = weakThis.get();
        if (!strongThis)
        {
            co_return;
        }

        FolderPicker picker;
        picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
        picker.FileTypeFilter().Append(L"*");

        // Initialize the picker with the window handle for desktop apps
        if (hwnd)
        {
            auto initializeWithWindow = picker.as<::IInitializeWithWindow>();
            if (initializeWithWindow)
            {
                initializeWithWindow->Initialize(hwnd);
            }
        }

        auto folder = co_await picker.PickSingleFolderAsync();

        strongThis = weakThis.get();
        if (!strongThis)
        {
            co_return;
        }

        if (folder)
        {
            strongThis->SelectedFolder(folder.Path());
        }
    }

    void LauncherPaneContent::_selectFolderClick(const IInspectable&, const RoutedEventArgs&)
    {
        // Get the foreground window handle for the picker
        HWND hwnd = GetForegroundWindow();
        _selectFolderAsync(get_weak(), hwnd);
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

    void LauncherPaneContent::UpdateGitLog(const winrt::hstring& workingDirectory)
    {
        _gitCommits.Clear();

        if (workingDirectory.empty())
        {
            _gitCommitsControl().ItemsSource(_gitCommits);
            return;
        }

        // Execute git log command via cmd.exe
        std::wstring cmdLine = L"cmd.exe /c git -C \"" + std::wstring(workingDirectory) + L"\" log --pretty=format:\"%ad  %h|%s\" --date=format:\"%Y-%m-%d %H:%M:%S\"";

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE hReadPipe, hWritePipe;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        {
            return;
        }

        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOW si = { sizeof(STARTUPINFOW) };
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi = {};

        std::wstring cmdBuffer = cmdLine;
        if (!CreateProcessW(nullptr, cmdBuffer.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, workingDirectory.c_str(), &si, &pi))
        {
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return;
        }

        CloseHandle(hWritePipe);

        // Read output
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

        // Parse output and populate git commits (convert UTF-8 to UTF-16)
        std::wstring wOutput;
        if (!output.empty())
        {
            int wideLen = MultiByteToWideChar(CP_UTF8, 0, output.c_str(), static_cast<int>(output.size()), nullptr, 0);
            if (wideLen > 0)
            {
                wOutput.resize(wideLen);
                MultiByteToWideChar(CP_UTF8, 0, output.c_str(), static_cast<int>(output.size()), wOutput.data(), wideLen);
            }
        }
        std::wistringstream stream(wOutput);
        std::wstring line;

        while (std::getline(stream, line))
        {
            if (line.empty())
            {
                continue;
            }

            // Find the separator between datetime+hash and message
            auto separatorPos = line.find(L'|');
            if (separatorPos != std::wstring::npos)
            {
                auto dateTimeAndHash = line.substr(0, separatorPos);
                auto message = line.substr(separatorPos + 1);
                _gitCommits.Append(winrt::make<GitCommitItem>(winrt::hstring(dateTimeAndHash), winrt::hstring(message)));
            }
        }

        // Set the ItemsSource on the control
        _gitCommitsControl().ItemsSource(_gitCommits);
    }
}
