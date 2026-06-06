#include "f18505/m18505.h"
QVector<double> m18505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
