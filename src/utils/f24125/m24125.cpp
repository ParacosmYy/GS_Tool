#include "f24125/m24125.h"
QVector<double> m24125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
