#include "f21845/m21845.h"
QVector<double> m21845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
