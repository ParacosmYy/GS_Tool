#include "m7992/m7992.h"
QVector<double> m7992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
