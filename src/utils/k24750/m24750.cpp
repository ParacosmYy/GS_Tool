#include "k24750/m24750.h"
QVector<double> m24750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
