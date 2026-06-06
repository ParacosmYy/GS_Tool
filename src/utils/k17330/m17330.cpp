#include "k17330/m17330.h"
QVector<double> m17330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
