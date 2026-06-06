#include "p25515/m25515.h"
QVector<double> m25515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
