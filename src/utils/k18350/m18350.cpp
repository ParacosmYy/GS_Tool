#include "k18350/m18350.h"
QVector<double> m18350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
