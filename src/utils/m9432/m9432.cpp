#include "m9432/m9432.h"
QVector<double> m9432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
