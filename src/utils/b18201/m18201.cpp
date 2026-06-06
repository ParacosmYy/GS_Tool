#include "b18201/m18201.h"
QVector<double> m18201::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
