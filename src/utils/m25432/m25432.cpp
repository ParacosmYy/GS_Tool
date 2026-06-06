#include "m25432/m25432.h"
QVector<double> m25432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
