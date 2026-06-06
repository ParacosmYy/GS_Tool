#include "r7817/m7817.h"
QVector<double> m7817::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
