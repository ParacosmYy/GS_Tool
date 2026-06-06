#include "l20911/m20911.h"
QVector<double> m20911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
