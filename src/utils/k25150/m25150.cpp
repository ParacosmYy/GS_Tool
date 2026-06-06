#include "k25150/m25150.h"
QVector<double> m25150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
