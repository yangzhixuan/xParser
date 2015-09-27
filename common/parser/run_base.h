#ifndef _RUN_H
#define _RUN_H

#include <string>

#include "depparser_base.h"

class RunBase {
public:
    RunBase() = default;

    virtual ~RunBase() = default;

    virtual void train(const std::string &sInputFile, const std::string &sFeatureInput,
                       const std::string &sFeatureOutput) const = 0;

    virtual void parse(const std::string &sInputFile, const std::string &sOutputFile,
                       const std::string &sFeatureFile) const = 0;

    virtual void goldtest(const std::string &sInputFile, const std::string &sFeatureInput) const = 0;
};

#endif