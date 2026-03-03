/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "cli.h"
#include "argparse/argparse.hpp"
#include "build/projectBuilder.h"
#include "utils/logger.h"
#include "editor/actions.h"

namespace
{
  std::string argProgPath{};
}

const std::string& CLI::getProjectPath()
{
  return argProgPath;
}

CLI::Result CLI::run(int argc, char** argv)
{
  //Windows will pass the project path as an argument when you double click a project file
  if (argc > 1) {
    std::string path = argv[1];
    if (path.ends_with(".p64proj")) {
      Editor::Actions::call(Editor::Actions::Type::PROJECT_OPEN, path);
      return Result::GUI;
    }
  }

  argparse::ArgumentParser prog{"pyrite64", PYRITE_VERSION};
  prog.add_argument("--cli")
   .help("Run in CLI mode (no GUI)")
   .default_value(false)
   .implicit_value(true);

  prog.add_argument("--cmd")
    .help("Command to run")
    .choices("build", "clean");

  prog.add_argument("project")
    .default_value("")
    .help("Path to project file (.p64proj)")
  ;

  argProgPath = {};
  try {
    prog.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << prog;
    return Result::ERROR;
  }

  argProgPath = prog.get<std::string>("project");

  if (prog["--cli"] == false) {
    return Result::GUI;
  }

  auto cmd = prog.get<std::string>("--cmd");

  Utils::Logger::setOutput([](const std::string &msg) {
    fputs(msg.c_str(), stdout);
  });

  printf("Pyrite64 - CLI\n");
  bool res = false;

  if (cmd == "build") {
    printf("Building project: %s\n", argProgPath.c_str());
    res = Build::buildProject(argProgPath);
  }
  else if (cmd == "clean")
  {
    printf("Cleaning project: %s\n", argProgPath.c_str());
    Project::Project project{argProgPath};
    res = Build::cleanProject(project, {
      .code = true,
      .assets = true,
      .engine = true,
    });
  }

  return res ? Result::SUCCESS : Result::ERROR;
}
