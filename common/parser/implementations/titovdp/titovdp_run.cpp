//
// Created by 杨至轩 on 9/3/15.
//

#include <ctime>
#include <memory>
#include <fstream>
#include <iostream>
#include "common/parser/macros_base.h"
#include "titovdp_run.h"
#include "titovdp_depparser.h"

namespace titovdp{

    Run::Run() = default;

    Run::~Run() = default;

    void Run::train(const std::string &sInputFile, const std::string &sFeatureInput,
                    const std::string &sFeatureOutput) const {
        int nRound = 0;
        DependencyGraph ref_sent;
        std::unique_ptr<DepParser> parser(new DepParser(sFeatureInput, sFeatureOutput, ParserState::TRAIN));
        std::ifstream input(sInputFile);

        std::cout << "training is started" << std::endl;
        size_t tic = clock();
        size_t toc;
        if (input) {
            while (input >> ref_sent) {
                ++nRound;
                parser->train(ref_sent);
                if(nRound % OUTPUT_STEP == 0) {
                    toc = clock();
                    double speed = (double)nRound / ((double)(toc - tic) / CLOCKS_PER_SEC);
                    std::cout << "Speed: " << speed << " sentences / second" << std::endl;
                    std::cout << std::endl;
                }
            }
            parser->finishtraining();
        }
        input.close();

        std::cout << "Done. " << nRound<< " sentences were processed"<<std::endl;
    }

    void Run::parse(const std::string &sInputFile, const std::string &sOutputFile,
                    const std::string &sFeatureFile) const {
    }

    void Run::goldtest(const std::string &sInputFile, const std::string &sFeatureInput) const {

    }

}
