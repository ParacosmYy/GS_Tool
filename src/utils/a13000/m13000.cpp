#include "a13000/m13000.h"
QVector<double> m13000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
