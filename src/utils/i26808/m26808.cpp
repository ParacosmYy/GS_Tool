#include "i26808/m26808.h"
QVector<double> m26808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
