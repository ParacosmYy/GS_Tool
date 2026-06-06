#include "s26878/m26878.h"
QVector<double> m26878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
