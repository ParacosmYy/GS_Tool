#include "k13330/m13330.h"
QVector<double> m13330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
