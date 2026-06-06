#include "i20808/m20808.h"
QVector<double> m20808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
