#include "k36590/m36590.h"
QVector<double> m36590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
