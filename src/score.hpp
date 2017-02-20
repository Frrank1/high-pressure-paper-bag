#ifndef HPPB_SCORE_FUNCTION_HPP
#define HPPB_SCORE_FUNCTION_HPP

#include <vector>
class Volume;

struct Split {
    float score;
    int index;
    int axis;
};

Split score(const std::vector<Volume>& shape);

#endif
