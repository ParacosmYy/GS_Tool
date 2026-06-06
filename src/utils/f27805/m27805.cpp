#include "f27805/m27805.h"
QVector<double> m27805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
