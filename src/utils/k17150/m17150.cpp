#include "k17150/m17150.h"
QVector<double> m17150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
