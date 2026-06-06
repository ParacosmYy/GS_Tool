#include "m8292/m8292.h"
QVector<double> m8292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
