#include "s7918/m7918.h"
QVector<double> m7918::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
