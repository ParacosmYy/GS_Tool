#include "e24204/m24204.h"
QVector<double> m24204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
