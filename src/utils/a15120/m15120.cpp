#include "a15120/m15120.h"
QVector<double> m15120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
