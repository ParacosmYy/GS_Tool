#include "e9164/m9164.h"
QVector<double> m9164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
