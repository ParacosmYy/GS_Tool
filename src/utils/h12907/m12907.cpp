#include "h12907/m12907.h"
QVector<double> m12907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
