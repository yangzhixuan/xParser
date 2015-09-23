//
// Created by 杨至轩 on 9/3/15.
//

#ifndef XPARSER_TITOVDP_RUN_H
#define XPARSER_TITOVDP_RUN_H

#include "common/parser/run_base.h"
namespace titovdp{
    class Run : public RunBase {
    public:
        Run();
        ~Run();

        void train(const std::string & sInputFile, const std::string & sFeatureInput, const std::string & sFeatureOutput) const override;
        void parse(const std::string & sInputFile, const std::string & sOutputFile, const std::string & sFeatureFile) const override;
        void goldtest(const std::string & sInputFile, const std::string & sFeatureInput) const override;

    };

}
#endif //XPARSER_TITOVDP_RUN_H
