#include "p24835/m24835.h"
QVector<double> m24835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
