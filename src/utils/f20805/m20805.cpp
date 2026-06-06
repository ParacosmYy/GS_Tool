#include "f20805/m20805.h"
QVector<double> m20805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
