#include "b25021/m25021.h"
QVector<double> m25021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
