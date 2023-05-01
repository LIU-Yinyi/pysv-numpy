#ifndef SVDPI_REGISTER
#define SVDPI_REGISTER

#include <utility>
#include <vector>
#include <string>

typedef std::pair<std::string, std::string> PyFileFunc;
typedef std::vector<PyFileFunc> PyFileFuncVec;

const PyFileFuncVec pyfilefunc_reg {
    {"payload", "callback"}
};

#endif /* SVDPI_REGISTER */