#include "e24284/m24284.h"
QVector<double> m24284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
