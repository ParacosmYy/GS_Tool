#include "g18146/m18146.h"
QVector<double> m18146::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
