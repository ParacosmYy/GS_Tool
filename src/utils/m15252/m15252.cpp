#include "m15252/m15252.h"
QVector<double> m15252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
