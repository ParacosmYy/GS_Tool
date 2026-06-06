#include "k15770/m15770.h"
QVector<double> m15770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
