#include "k10190/m10190.h"
QVector<double> m10190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
