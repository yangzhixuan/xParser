#include "common/parser/run_base.h"

namespace eisner {
    class Run : public RunBase {
    public:
        Run();

        ~Run();

        void train(const std::string &sInputFile, const std::string &sFeatureInput,
                   const std::string &sFeatureOutput) const override;

        void parse(const std::string &sInputFile, const std::string &sOutputFile,
                   const std::string &sFeatureFile) const override;

        void goldtest(const std::string &sInputFile, const std::string &sFeatureInput) const override;
    };
}