#include "d9883/m9883.h"
QVector<double> m9883::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
