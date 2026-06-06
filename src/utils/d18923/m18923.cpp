#include "d18923/m18923.h"
QVector<double> m18923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
