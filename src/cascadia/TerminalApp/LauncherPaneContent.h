// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once
#include "LauncherPaneContent.g.h"
#include "LaunchRequestedArgs.g.h"
#include "GitCommitItem.g.h"
#include "BasicPaneEvents.h"
#include <functional>

namespace winrt::TerminalApp::implementation
{
    struct LaunchRequestedArgs : LaunchRequestedArgsT<LaunchRequestedArgs>
    {
        LaunchRequestedArgs() = default;
        WINRT_PROPERTY(winrt::hstring, WorkingDirectory);
        WINRT_PROPERTY(winrt::hstring, Command);
    };

    struct GitCommitItem : GitCommitItemT<GitCommitItem>
    {
        GitCommitItem() = default;
        GitCommitItem(winrt::hstring dateTimeAndHash, winrt::hstring message)
            : _dateTimeAndHash(std::move(dateTimeAndHash)), _message(std::move(message)) {}

        winrt::hstring DateTimeAndHash() const { return _dateTimeAndHash; }
        winrt::hstring Message() const { return _message; }

    private:
        winrt::hstring _dateTimeAndHash;
        winrt::hstring _message;
    };

    struct TerminalPage;

    struct LauncherPaneContent : LauncherPaneContentT<LauncherPaneContent>, BasicPaneEvents
    {
        friend struct winrt::TerminalApp::implementation::TerminalPage;
    public:
        LauncherPaneContent();

        winrt::Windows::UI::Xaml::FrameworkElement GetRoot();

        void UpdateSettings(const winrt::Microsoft::Terminal::Settings::Model::CascadiaSettings& settings);

        winrt::Windows::Foundation::Size MinimumSize();
        void Focus(winrt::Windows::UI::Xaml::FocusState reason = winrt::Windows::UI::Xaml::FocusState::Programmatic);
        void Close();
        winrt::Microsoft::Terminal::Settings::Model::INewContentArgs GetNewTerminalArgs(BuildStartupKind kind) const;

        winrt::hstring Title() { return L"AI 代码助手"; }
        uint64_t TaskbarState() { return 0; }
        uint64_t TaskbarProgress() { return 0; }
        bool ReadOnly() { return false; }
        winrt::hstring Icon() const;
        Windows::Foundation::IReference<winrt::Windows::UI::Color> TabColor() const noexcept { return nullptr; }
        winrt::Windows::UI::Xaml::Media::Brush BackgroundBrush();

        til::property_changed_event PropertyChanged;
        WINRT_OBSERVABLE_PROPERTY(winrt::hstring, SelectedFolder, PropertyChanged.raise, L"");

        // Git log update
        void UpdateGitLog(const winrt::hstring& workingDirectory);

        // Event for launch requests
        til::typed_event<TerminalApp::LauncherPaneContent, TerminalApp::LaunchRequestedArgs> LaunchRequested;

    private:
        friend struct LauncherPaneContentT<LauncherPaneContent>;

        winrt::Microsoft::Terminal::Settings::Model::CascadiaSettings _settings{ nullptr };
        winrt::Windows::Foundation::Collections::IObservableVector<TerminalApp::GitCommitItem> _gitCommits{ nullptr };

        void _selectFolderClick(const Windows::Foundation::IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs&);
        void _claudeClick(const Windows::Foundation::IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs&);
        void _codexClick(const Windows::Foundation::IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs&);
        void _geminiClick(const Windows::Foundation::IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs&);

        void _launchWithCommand(const winrt::hstring& command);
    };
}

namespace winrt::TerminalApp::factory_implementation
{
    BASIC_FACTORY(LauncherPaneContent);
    BASIC_FACTORY(LaunchRequestedArgs);
    BASIC_FACTORY(GitCommitItem);
}
