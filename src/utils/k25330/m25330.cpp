#include "k25330/m25330.h"
QVector<double> m25330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
