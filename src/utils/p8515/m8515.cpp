#include "p8515/m8515.h"
QVector<double> m8515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
