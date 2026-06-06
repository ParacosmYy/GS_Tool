#include "a29900/m29900.h"
QVector<double> m29900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
