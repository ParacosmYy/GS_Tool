#include "m18932/m18932.h"
QVector<double> m18932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
