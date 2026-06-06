#include "l24711/m24711.h"
QVector<double> m24711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
