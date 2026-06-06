#include "k9330/m9330.h"
QVector<double> m9330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
