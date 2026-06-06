#include "a18520/m18520.h"
QVector<double> m18520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
