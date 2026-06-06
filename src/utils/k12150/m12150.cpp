#include "k12150/m12150.h"
QVector<double> m12150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
