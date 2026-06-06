#include "i9308/m9308.h"
QVector<double> m9308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
