#include "k25130/m25130.h"
QVector<double> m25130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
