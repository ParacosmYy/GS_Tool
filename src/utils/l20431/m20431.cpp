#include "l20431/m20431.h"
QVector<double> m20431::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
