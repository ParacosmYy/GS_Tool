#include "i26008/m26008.h"
QVector<double> m26008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
