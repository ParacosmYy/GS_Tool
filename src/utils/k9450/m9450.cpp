#include "k9450/m9450.h"
QVector<double> m9450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
