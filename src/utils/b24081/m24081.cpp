#include "b24081/m24081.h"
QVector<double> m24081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
