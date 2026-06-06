#include "l16931/m16931.h"
QVector<double> m16931::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
