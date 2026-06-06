#include "b25741/m25741.h"
QVector<double> m25741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
