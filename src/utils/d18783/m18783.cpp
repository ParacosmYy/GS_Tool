#include "d18783/m18783.h"
QVector<double> m18783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
