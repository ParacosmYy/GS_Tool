#include "o25054/m25054.h"
QVector<double> m25054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
