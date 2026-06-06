#include "d18563/m18563.h"
QVector<double> m18563::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
