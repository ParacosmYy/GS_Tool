#include "p18535/m18535.h"
QVector<double> m18535::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
