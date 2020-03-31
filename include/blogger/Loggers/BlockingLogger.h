#pragma once

#include "blogger/Loggers/Logger.h"
#include "blogger/LogLevels.h"
#include "blogger/Sinks/Sink.h"

namespace bl {

    class BlockingLogger : public Logger
    {
    public:
        BlockingLogger(
            InString tag,
            level lvl,
            bool default_pattern = true
        ) : Logger(
            tag,
            lvl,
            default_pattern
        )
        {
        }

        void Flush() override
        {
            for (auto& sink : *m_Sinks)
            {
                sink->flush();
            }
        }
    private:
        void Post(BLoggerLogMessage&& msg) override
        {
            msg.finalize_format();

            for (auto& sink : *m_Sinks)
            {
                sink->write(msg);
            }
        }
    };
}