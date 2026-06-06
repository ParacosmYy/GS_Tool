#include "f9005/m9005.h"
QVector<double> m9005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
