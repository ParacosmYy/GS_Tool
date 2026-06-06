#include "a15200/m15200.h"
QVector<double> m15200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
