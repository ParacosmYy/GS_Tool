#include "s9558/m9558.h"
QVector<double> m9558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
