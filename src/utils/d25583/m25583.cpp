#include "d25583/m25583.h"
QVector<double> m25583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
