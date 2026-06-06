#include "o25014/m25014.h"
QVector<double> m25014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
