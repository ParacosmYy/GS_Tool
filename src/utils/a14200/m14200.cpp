#include "a14200/m14200.h"
QVector<double> m14200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
