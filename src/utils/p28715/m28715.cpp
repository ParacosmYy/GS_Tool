#include "p28715/m28715.h"
QVector<double> m28715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
