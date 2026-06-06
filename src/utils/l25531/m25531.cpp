#include "l25531/m25531.h"
QVector<double> m25531::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
