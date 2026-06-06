#include "g9066/m9066.h"
QVector<double> m9066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
