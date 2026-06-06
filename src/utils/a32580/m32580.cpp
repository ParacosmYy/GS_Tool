#include "a32580/m32580.h"
QVector<double> m32580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
