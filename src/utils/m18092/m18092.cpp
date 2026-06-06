#include "m18092/m18092.h"
QVector<double> m18092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
