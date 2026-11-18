#include "maix_basic.hpp"
#include "main.h"

using namespace maix;

int _main(int argc, char *argv[])
{
    const uint64_t start_time = time::time_s();

    log::info("Hello World!");
    log::info("Hello from MaixCAM Pro via MaixCDK.");
    log::info("app_id: %s", app::app_id().c_str());

    // 保持主循环，直到 Ctrl+C、USER 键或系统切换应用请求退出。
    while (!app::need_exit())
    {
        log::info("running for %llu s", (unsigned long long)(time::time_s() - start_time));
        time::sleep(1);

        // 10 秒后自动退出，方便第一次运行时确认程序能正常收尾。
        if (time::time_s() - start_time >= 10)
            app::set_exit_flag(true);
    }

    log::info("Program exit.");
    return 0;
}

int main(int argc, char *argv[])
{
    // 注册 Ctrl+C 等信号处理，确保 MaixCDK 资源可以正常释放。
    sys::register_default_signal_handle();

    // 捕获 MaixCDK 异常，避免设备资源未析构就退出。
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}