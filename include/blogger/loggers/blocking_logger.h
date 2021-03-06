#pragma once

#include "blogger/loggers/logger.h"
#include "blogger/log_levels.h"
#include "blogger/sinks/sink.h"

namespace bl {

    class blocking_logger : public logger
    {
    public:
        blocking_logger(
            in_string tag,
            level lvl,
            bool default_pattern = true
        ) : logger(tag, lvl, default_pattern)
        {
        }

        void flush() override
        {
            for (auto& sink : *m_sinks)
            {
                sink->flush();
            }
        }
    private:
        void post(log_message&& msg) override
        {
            msg.finalize_format();

            for (auto& sink : *m_sinks)
            {
                sink->write(msg);
            }
        }
    };
}
