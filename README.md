This repo showcases failures when multiple windows are opened around the same time with the Windows App SDK v1.5.

## Repro steps
1. Clone repo.
2. Open multi-window-hang.sln.
3. Build and launch the app (issue seems to repro in both Debug & Release). I had to restart VS after building the app.
4. In the app, pressing the red button will lead to either an unexpected wrong thread exception or one of the windows will open successfully while the other fails to.
   
The issue doesn't repro if the window setup code in the DispatcherQueue callback to the Application::Start or the App::OnLaunched callbacks.
