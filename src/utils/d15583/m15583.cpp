#include "d15583/m15583.h"
QVector<double> m15583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
