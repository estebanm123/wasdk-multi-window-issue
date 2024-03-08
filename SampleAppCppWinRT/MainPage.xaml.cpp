#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include <format>

#include <ppltasks.h>
#include <pplawait.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::SampleAppCppWinRT::implementation
{
    MainPage::MainPage() {}

    struct WindowData
    {
        Microsoft::UI::Dispatching::DispatcherQueue dq = nullptr;
        Microsoft::UI::Xaml::Window window = nullptr;
    };

    template<typename... Args>
    void Log(std::string input, Args&&... args)
    {
        auto formatStr = std::string("!~ thread {}: ") + input;
        auto str = std::vformat(formatStr, std::make_format_args(GetCurrentThreadId(), args...)) + "\n";
        OutputDebugStringA(str.c_str());
    }

    Windows::Foundation::IAsyncOperation<Microsoft::UI::Xaml::Window> CreateNewCppWindowOnNewThread()
    {
        auto windowCreationEvent = concurrency::task_completion_event<void>();
        auto task = concurrency::create_task(windowCreationEvent);

        auto windowData = std::make_shared<WindowData>();
        auto thread = std::thread([=]()
            {
                Log("Created new UI thread");

                winrt::init_apartment(winrt::apartment_type::single_threaded);

                Microsoft::UI::Xaml::Application::Start([=](auto&&)
                    {
                        Log("Application::Start callback started");

                        windowData->dq = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
                        windowData->window = Microsoft::UI::Xaml::Window();
                        auto window = windowData->window;

                        windowCreationEvent.set();

                        Log("Application::Start callback ended");
                    }
                );

                Log("Thread exiting");
            });

        thread.detach();

        Log("Wait for completion event");
        co_await task;

        Log("Enqueing window setup");
        windowData->dq.TryEnqueue(
        [windowData]()
        {
            Log("Setting up window. Does the DQ have thread access? {}", windowData->dq.HasThreadAccess());

            auto window = windowData->window;
            window.Title(L"SecondaryWindow");
            window.Content(UICppWinRT::MyPageCppWinRT());
            window.Activate();

            Log("Window setup done");
        });

        co_return nullptr;
    }

    Windows::Foundation::IAsyncAction MainPage::CreateSingleCppWindowOnNewThreadClicked(Windows::Foundation::IInspectable const&, RoutedEventArgs const&)
    {
         CreateNewCppWindowOnNewThread(); 
        co_return;
    }

    Windows::Foundation::IAsyncAction MainPage::Create2CppWindowsOnNewThreadClicked(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args)
    {
        co_await winrt::resume_background();

        CreateNewCppWindowOnNewThread();
        CreateNewCppWindowOnNewThread();
        co_return;
    }

}
