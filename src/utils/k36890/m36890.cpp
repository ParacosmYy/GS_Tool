#include "k36890/m36890.h"
QVector<double> m36890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
