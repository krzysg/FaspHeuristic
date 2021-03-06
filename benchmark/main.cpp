#include "tools/easylogging++.h"
#include "tools/prettyprint.h"
#include "tclap/CmdLine.h"

#include "benchmarks/benchIsoCut/benchIsoCut.h"
#include "benchmarks/benchHeuristics/benchHeuristics.h"
#include "benchmarks/benchGraphsFromPaper/benchGraphsFromPaper.h"
#include "benchmarks/benchILPvsHEURISTIC/benchILPvsHEURISTIC.h"


INITIALIZE_EASYLOGGINGPP

/**
 * Configuration for easy logging.
 */
void configureLogger() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
    defaultConf.set(el::Level::Global, el::ConfigurationType::SubsecondPrecision, "1");
    constexpr const char *format = "[%datetime, %fbase:%line] %msg";
    defaultConf.set(el::Level::Global, el::ConfigurationType::Format, format);
    el::Loggers::reconfigureLogger("default", defaultConf);
}

/**
 * Checks if mandatory argument for benchmark was set. If no, complain to user and exit.
 * @return value of argument if it was set correctly
 */
template <typename T>
auto reqArgHdl(TCLAP::ValueArg<T> &arg) {
    if (!arg.isSet()) {
        LOG(ERROR) << "Argument: [" << arg.longID("") << " " << arg.getDescription() << "] is required!";
        exit(-1);
    }
    return arg.getValue();
}

/**
 * Simple code for running benchmarks from command line.
 *
 * NOTICE: This code is kept as easy as possible and it's not perfect.
 * So it defines all needed arguments and verifies *later* (after parsing the command line) which field is mandatory for chosen
 * benchmark.
 */
int main(int argc, char **argv) {
    configureLogger();

    try {
        TCLAP::CmdLine cmd("FASP heuristic benchamarks", ' ', "1.0", true);

        // Helper fields - they will be checked later on per-benchmark basis (each benchmark may req. different set of those fields)
        TCLAP::ValueArg<std::string> dirArg("d", "outputDirectory", "directory where output files will be saved", false, "", "outputDirectory");
        TCLAP::ValueArg<std::string> filenameArg("n", "fileName", "name of the output file", false, "", "outputDirectory");

        TCLAP::ValueArg<std::string> dirInArg("i", "inputDirectory", "directory where graphs are stored", false, "", "inputDirectory");

        TCLAP::ValueArg<int> vArg("y", "v", "number of vertices in graph", false, 0, "#vertices");
        TCLAP::ValueArg<int> vMinArg("b", "vmin", "min (begin) number of vertices in graph", false, 0, "#minNumberOfVertices");
        TCLAP::ValueArg<int> vMaxArg("e", "vmax", "max (end) number of vertices in graph", false, 0, "#maxNumberOfVertices");

        TCLAP::ValueArg<int> eArg("w", "e", "number of vertices in graph", false, 0, "#edges");
        TCLAP::ValueArg<int> eMinArg("s", "emin", "min (begin) number of edges in graph", false, 0, "#minNumberOfEdges");
        TCLAP::ValueArg<int> eMaxArg("t", "emax", "max (end) number of edges in graph", false, 0, "#maxNumberOfEdges");

        TCLAP::ValueArg<double> dArg("u", "d", "density of graph - ratio e/v", false, 0, "density");
        TCLAP::ValueArg<int> dMinArg("j", "dmin", "min (begin) density of graph - ratio e/v", false, 0, "minDensity");
        TCLAP::ValueArg<int> dMaxArg("k", "dmax", "max (end) density of graph - ratio e/v", false, 0, "maxDensity");

        TCLAP::ValueArg<int> fArg("f", "f", "size of FASP solution in graph", false, 0, "#faspSize");
        TCLAP::ValueArg<int> fMinArg("m", "fmin", "min (begin) size of FASP solution in graph", false, 0, "#minFaspSize");
        TCLAP::ValueArg<int> fMaxArg("x", "fmax", "max (end) size of FASP solution in graph", false, 0, "#maxFaspSize");

        TCLAP::ValueArg<int> stepsArg("z", "steps", "number of steps to be taken in given range of v/e/f", false, 0, "#steps");
        TCLAP::ValueArg<int> repsArg("r", "reps", "number of repetitions for each step", false, 0, "#repetitions");

        TCLAP::SwitchArg logArg("l", "logScale", "Use log distributed range", false);

        cmd.add(dirArg);
        cmd.add(filenameArg);
        cmd.add(dirInArg);

        cmd.add(vArg);
        cmd.add(vMinArg);
        cmd.add(vMaxArg);

        cmd.add(eArg);
        cmd.add(eMinArg);
        cmd.add(eMaxArg);

        cmd.add(dArg);
        cmd.add(dMinArg);
        cmd.add(dMaxArg);

        cmd.add(fArg);
        cmd.add(fMinArg);
        cmd.add(fMaxArg);

        cmd.add(stepsArg);
        cmd.add(repsArg);

        cmd.add(logArg);

        // Mandatory field - define new benchmarks.
        std::string ilpVsHeuristic{"benchILPvsHEURISTIC"};
        std::string isoCutConstWeightVarFaspConstVE{"benchIsoCutConstWeightVarFaspConstVE"};
        std::string graphsFromPaper1{"benchGraphsFromPaper"};
        std::string varFaspConstWeightVE{"benchVarFaspConstWeightVE"};
        std::string varVConstDensityFaspWeight{"benchVarVConstDensityFaspWeight"};
        std::string varWeightFaspConstVE{"benchVarWeightFaspConstVE"};

        std::vector<std::string> allowedBenchmarks{ilpVsHeuristic, isoCutConstWeightVarFaspConstVE, graphsFromPaper1, varFaspConstWeightVE, varVConstDensityFaspWeight, varWeightFaspConstVE};
        TCLAP::ValuesConstraint<std::string> allowedVals( allowedBenchmarks );
        TCLAP::UnlabeledValueArg<std::string>  benchmarkName("benchmarkName", "name of benchmark to run", true, "", &allowedVals);
        cmd.add(benchmarkName);

        // ---------- PARSER ------------------
        cmd.parse( argc, argv );


        // run chosen benchmark
        if (benchmarkName.getValue() == ilpVsHeuristic) {
            benchILPvsHEURISTIC(reqArgHdl(dirInArg), reqArgHdl(dirArg), reqArgHdl((filenameArg)));
        }
        else if (benchmarkName.getValue() == isoCutConstWeightVarFaspConstVE) {
            benchIsoCutConstWeightVarFaspConstVE(reqArgHdl(dirArg), reqArgHdl(vArg), reqArgHdl(eArg), reqArgHdl(fMinArg), reqArgHdl(fMaxArg), reqArgHdl(stepsArg), reqArgHdl(repsArg), logArg.getValue());
        }
        else if (benchmarkName.getValue() == graphsFromPaper1) {
            benchGraphsFromPaper1(reqArgHdl(dirArg), reqArgHdl(dirInArg));
        }
        else if (benchmarkName.getValue() == varFaspConstWeightVE) {
            benchVarFaspConstWeightVE(reqArgHdl(dirArg), reqArgHdl(vArg), reqArgHdl(eArg), reqArgHdl(fMinArg), reqArgHdl(fMaxArg), reqArgHdl(stepsArg), reqArgHdl(repsArg), logArg.getValue());
        }
        else if (benchmarkName.getValue() == varVConstDensityFaspWeight) {
            benchVarVConstDensityFaspWeight(reqArgHdl(dirArg), reqArgHdl(vMinArg), reqArgHdl(vMaxArg), reqArgHdl(dArg),
                                            reqArgHdl(fArg), reqArgHdl(stepsArg), reqArgHdl(repsArg), logArg.getValue());
        }
        else if (benchmarkName.getValue() == varWeightFaspConstVE) {
            benchVarWeightFaspConstVE(reqArgHdl(dirArg), reqArgHdl(vArg), reqArgHdl(eArg), reqArgHdl(fMinArg),
                                      reqArgHdl(fMaxArg), reqArgHdl(stepsArg), reqArgHdl(repsArg), logArg.getValue());
        }
        else {
            std::cerr << "Not known name of benchmark! \n";
        }
    }
    catch (TCLAP::ArgException &e) {
        LOG(ERROR) << "Cmd line error: " << e.error() << " for arg " << e.argId();
        return -1;
    }

    return 0;
}
