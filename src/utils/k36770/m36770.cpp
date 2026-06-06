#include "k36770/m36770.h"
QVector<double> m36770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
