#include "c25582/m25582.h"
QVector<double> m25582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
