#include "b18841/m18841.h"
QVector<double> m18841::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
