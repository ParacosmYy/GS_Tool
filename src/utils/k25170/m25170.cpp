#include "k25170/m25170.h"
QVector<double> m25170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
