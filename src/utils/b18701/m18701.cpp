#include "b18701/m18701.h"
QVector<double> m18701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
