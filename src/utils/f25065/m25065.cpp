#include "f25065/m25065.h"
QVector<double> m25065::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
