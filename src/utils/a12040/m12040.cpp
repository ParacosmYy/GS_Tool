#include "a12040/m12040.h"
QVector<double> m12040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
