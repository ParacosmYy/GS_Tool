#include "k35990/m35990.h"
QVector<double> m35990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
