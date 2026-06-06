#include "m32932/m32932.h"
QVector<double> m32932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
