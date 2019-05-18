#include <atomic>
#include <cstdint>
#include <stack>

#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <logging.hpp>

namespace attributes = boost::log::attributes;


static std::atomic_uint_fast64_t procedure_counter(0);
static thread_local std::stack<std::uint64_t> procedure_stack;


namespace _logging_internal {
    BOOST_LOG_GLOBAL_LOGGER_INIT(logger, boost::log::sources::logger_mt)
    {
        boost::log::sources::logger_mt lg;
        return lg;
    }
}


void logging_init()
{
    auto core = boost::log::core::get();

    // Add basic global attributes
    core->add_global_attribute(
            logattr::line_id.get_name(),
            attributes::counter<unsigned int>(1));
    core->add_global_attribute(
            logattr::timestamp.get_name(),
            attributes::utc_clock());
    core->add_global_attribute(
            logattr::process_id.get_name(),
            attributes::current_process_id());
    core->add_global_attribute(
            logattr::thread_id.get_name(),
            attributes::current_thread_id());
    // Add default attribute values
    core->add_global_attribute(
            logattr::record_type.get_name(),
            attributes::make_constant(logrecord_type_e::info));
    // Add attribute about current procedure
    core->add_global_attribute(
            logattr::procedure.get_name(),
            attributes::make_function([]{ return logging_procedure_get(); }));
}

uint64_t logging_procedure_get()
{
    return procedure_stack.empty() ? 0 : procedure_stack.top();
}

uint64_t logging_procedure_push()
{
    procedure_stack.push(++procedure_counter);
    return procedure_stack.top();
}

uint64_t logging_procedure_pop()
{
    if (procedure_stack.empty())
        return 0;
    std::uint64_t removed = procedure_stack.top();
    procedure_stack.pop();
    return removed;
}
