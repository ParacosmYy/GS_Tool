#include "h25207/m25207.h"
QVector<double> m25207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
