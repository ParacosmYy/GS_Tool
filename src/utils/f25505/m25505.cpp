#include "f25505/m25505.h"
QVector<double> m25505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
