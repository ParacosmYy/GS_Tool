#include "k10050/m10050.h"
QVector<double> m10050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
