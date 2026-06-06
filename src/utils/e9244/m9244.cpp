#include "e9244/m9244.h"
QVector<double> m9244::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
