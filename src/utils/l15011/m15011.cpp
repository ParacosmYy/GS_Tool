#include "l15011/m15011.h"
QVector<double> m15011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
