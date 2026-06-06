#include "b24121/m24121.h"
QVector<double> m24121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
