#include "a7900/m7900.h"
QVector<double> m7900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
