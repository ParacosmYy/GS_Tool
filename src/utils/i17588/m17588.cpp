#include "i17588/m17588.h"
QVector<double> m17588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
