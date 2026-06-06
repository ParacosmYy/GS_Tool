#include "i35408/m35408.h"
QVector<double> m35408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
