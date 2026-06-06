#include "k25110/m25110.h"
QVector<double> m25110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
