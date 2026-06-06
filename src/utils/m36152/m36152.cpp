#include "m36152/m36152.h"
QVector<double> m36152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
