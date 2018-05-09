struct cmdname_help
{
    char name[16];
    char help[80];
};

static struct cmdname_help common_cmds[] = {
  {"annotate", "Read perf.data (created by perf record) and display annotated code"},
  {"archive", "Create archive with object files with build-ids found in perf.data file"},
  {"bench", "General framework for benchmark suites"},
  {"buildid-cache", "Manage build-id cache."},
  {"buildid-list", "List the buildids in a perf.data file"},
  {"diff", "Read perf.data files and display the differential profile"},
  {"evlist", "List the event names in a perf.data file"},
  {"inject", "Filter to augment the events stream with additional information"},
  {"kmem", "Tool to trace/measure kernel memory(slab) properties"},
  {"kvm", "Tool to trace/measure kvm guest os"},
  {"list", "List all symbolic event types"},
  {"lock", "Analyze lock events"},
  {"mem", "Profile memory accesses"},
  {"record", "Run a command and record its profile into perf.data"},
  {"report", "Read perf.data (created by perf record) and display the profile"},
  {"sched", "Tool to trace/measure scheduler properties (latencies)"},
  {"script", "Read perf.data (created by perf record) and display trace output"},
  {"stat", "Run a command and gather performance counter statistics"},
  {"test", "Runs sanity tests."},
  {"timechart", "Tool to visualize total system behavior during a workload"},
  {"top", "System profiling tool."},
  {"trace", "strace inspired tool"},
#ifdef LIBELF_SUPPORT
  {"probe", "Define new dynamic tracepoints"},
#endif 
};
