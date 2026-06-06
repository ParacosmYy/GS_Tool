#include "a16040/m16040.h"
QVector<double> m16040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
