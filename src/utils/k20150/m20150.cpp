#include "k20150/m20150.h"
QVector<double> m20150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
