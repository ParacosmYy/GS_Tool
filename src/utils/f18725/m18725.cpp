#include "f18725/m18725.h"
QVector<double> m18725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
