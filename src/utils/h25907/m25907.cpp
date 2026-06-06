#include "h25907/m25907.h"
QVector<double> m25907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
