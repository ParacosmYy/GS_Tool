#include "g18066/m18066.h"
QVector<double> m18066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
