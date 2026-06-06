#include "h18907/m18907.h"
QVector<double> m18907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
