#include "d18023/m18023.h"
QVector<double> m18023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
