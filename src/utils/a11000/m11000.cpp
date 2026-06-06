#include "a11000/m11000.h"
QVector<double> m11000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
