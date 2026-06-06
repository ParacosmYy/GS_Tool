#include "e18024/m18024.h"
QVector<double> m18024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
