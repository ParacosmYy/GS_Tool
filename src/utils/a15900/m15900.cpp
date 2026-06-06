#include "a15900/m15900.h"
QVector<double> m15900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
