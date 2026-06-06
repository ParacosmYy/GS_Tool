#include "m18992/m18992.h"
QVector<double> m18992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
