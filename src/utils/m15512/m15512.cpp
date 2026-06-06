#include "m15512/m15512.h"
QVector<double> m15512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
