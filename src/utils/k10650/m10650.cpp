#include "k10650/m10650.h"
QVector<double> m10650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
