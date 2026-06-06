#include "a26040/m26040.h"
QVector<double> m26040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
