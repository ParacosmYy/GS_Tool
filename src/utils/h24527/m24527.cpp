#include "h24527/m24527.h"
QVector<double> m24527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
