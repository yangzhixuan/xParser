#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "common/parser/implementations/titovdp/titovdp_run.h"
#include "common/parser/implementations/eisner/eisner_run.h"
#include "common/parser/implementations/eisnergc/eisnergc_run.h"
#include "common/parser/implementations/eisner3rd/eisner3rd_run.h"
#include "common/parser/implementations/eisnergc3rd/eisnergc3rd_run.h"
#include "common/parser/implementations/emptyeisner/emptyeisner_run.h"
#include "common/parser/implementations/emptyeisnergc3rd/emptyeisnergc3rd_run.h"

int main(int argc, char *argv[]) {
    std::unique_ptr<RunBase> run(nullptr);

    if (strcmp(argv[2], "eisner") == 0) {
        run.reset(new eisner::Run());
    }
    else if (strcmp(argv[2], "eisner3rd") == 0) {
        run.reset(new eisner3rd::Run());
    }
    else if (strcmp(argv[2], "eisnergc") == 0) {
        run.reset(new eisnergc::Run());
    }
    else if (strcmp(argv[2], "eisnergc3rd") == 0) {
        run.reset(new eisnergc3rd::Run());
    }
    else if (strcmp(argv[2], "emptyeisner") == 0) {
        run.reset(new emptyeisner::Run());
    }
    else if (strcmp(argv[2], "emptyeisnergc3rd") == 0) {
        run.reset(new emptyeisnergc3rd::Run());
    }
    else if (strcmp(argv[2], "titovdp") == 0) {
        run.reset(new titovdp::Run());
    }

    if (strcmp(argv[1], "goldtest") == 0) {
        run->goldtest(argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "train") == 0) {

        int iteration = std::atoi(argv[5]);

        std::string current_feature;
        std::string next_feature;

        current_feature = next_feature = argv[4];
        next_feature = next_feature.substr(0, next_feature.rfind("\\") + strlen("\\")) + argv[2] + "1.feat";

        for (int i = 0; i < iteration; ++i) {
            run->train(argv[3], current_feature, next_feature);
            current_feature = next_feature;
            next_feature =
                    next_feature.substr(0, next_feature.rfind(argv[2]) + strlen(argv[2])) + std::to_string(i + 2) +
                    ".feat";
        }
    }
    else if (strcmp(argv[1], "parse") == 0) {
        run->parse(argv[3], argv[5], argv[4]);
    }
    return 0;
}