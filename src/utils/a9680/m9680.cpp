#include "a9680/m9680.h"
QVector<double> m9680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
