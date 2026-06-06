#include "a14000/m14000.h"
QVector<double> m14000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
