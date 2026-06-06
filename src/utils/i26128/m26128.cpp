#include "i26128/m26128.h"
QVector<double> m26128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
