#include "m32292/m32292.h"
QVector<double> m32292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
