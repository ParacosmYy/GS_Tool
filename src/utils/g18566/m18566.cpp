#include "g18566/m18566.h"
QVector<double> m18566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
