#include "p16515/m16515.h"
QVector<double> m16515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
